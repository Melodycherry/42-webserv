/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                        LES CODEUSES DU DIMANCHE                           */
/*                                FONT UN                                    */
/*                                                                           */
/*                   _ _ _ ____ ___  ____ ____ ____ _  _                     */
/*                   | | | |___ |__] [__  |___ |__/ |  |                     */
/*                   |_|_| |___ |__] ___] |___ |  \  \/                      */
/*                                                                           */
/*****************************************************************************/

#include "WebServer.hpp"

/* ************************************************** */
/* construtor & destructors                           */
/* ************************************************** */

WebServer::WebServer(const std::vector<ServerConfig> &servers) : _servers(servers), _running(true) {
	initSockets();
	cpyLinkConfig();
}

WebServer::~WebServer(){
	for (std::map<int, SocketClient*>::iterator it = _socketClients.begin();
		 it != _socketClients.end(); ++it) {
		
		close(it->first);
		delete it->second;
	}
	_socketClients.clear();

	for (std::vector<SocketServer*>::iterator it = _socketServers.begin();
		it != _socketServers.end(); ++it) {
	delete *it;
	}
	_socketServers.clear();
}

/* ************************************************** */
/* POLL LOOP			                              */
/* ************************************************** */

void WebServer::pollLoop() {
    initPollStruct();
    while (_running) {

        int ret = poll(&_pollFds[0], _pollFds.size(), 1000);

        if (ret == -1) {
            if (errno == EINTR) {
                _running = false;
                break;
            } else {
                throw RunningException(std::string("Poll: ") + strerror(errno));
            }
        }
        
		checkTimeouts();
		
		if (ret == 0)
            continue;

        for (size_t i = 0; i < _pollFds.size(); ) {

            int fd = _pollFds[i].fd;
            short revents = _pollFds[i].revents;

			// EVAL :Un seul POLLIN  donc un seul handleRequest() ou acceptClient()
            if (revents & POLLIN) {

                if (isServerFd(fd)) {
                    try {
                        acceptClient(fd);
                    } catch (const ResponseException& e) {
                        sendResponse(e.getFd());
                    }
                } else {
                    try {
                        handleRequest(fd);
                    } catch (const ResponseException& e) {
                        sendResponse(e.getFd());
                    }
                }
            }

			// EVAL : Un seul POLLOUT donc un seul sendResponse()
            if (revents & POLLOUT) {

                try {
                    sendResponse(fd);
                } catch (...) {
                    closeConnection(fd);
					// 500
                    continue;
                }
            }

			if (revents & (POLLHUP | POLLERR)) {
				std::map<int, SocketClient*>::iterator it = _socketClients.find(fd);
				if (it == _socketClients.end())
    				continue;
				SocketClient* client = it->second;

				if (client && client->state == PROCESSING) {
					setPollOut(fd);
					i++;
					continue;
				}
				
				closeConnection(fd);
                continue;
            }
            i++;
        }
    }
}

void	WebServer::acceptClient(int serverFd){
	LOG("Accept client on server fd " << serverFd);

	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	int clientFd = accept(serverFd, (struct sockaddr*)&addr, &addrlen);
	if (clientFd < 0)
		return ;
	
	if (_socketClients.size() >= 1024) {
		earlyError(503, clientFd);
		return ;
	}

	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags < 0)
		throw ResponseException(clientFd, 500);
	if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw ResponseException(clientFd, 500);
	

	int fdFlags = fcntl(clientFd, F_GETFD);
	if (fdFlags < 0)
		throw ResponseException(clientFd, 500);
	if (fcntl(clientFd, F_SETFD, fdFlags | FD_CLOEXEC) < 0)
		throw ResponseException(clientFd, 500);

	SocketServer* serverPtr = NULL;
    for (size_t i = 0; i < _socketServers.size(); ++i) {
        if (_socketServers[i]->getFd() == serverFd) {
            serverPtr = _socketServers[i];
            break;
        }
    }
    if (!serverPtr){
        throw ResponseException(clientFd, 500);
	}
	
	SocketClient* client = new SocketClient(clientFd, addr, serverPtr);
	client->lastActivity = std::time(NULL);

    _socketClients[clientFd] = client;

	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
	
	LOG("New client fd = " << clientFd);
}

void	WebServer::handleRequest(int fd){
	LOG("HandleRequest fd = " << fd);
	SocketClient* client = _socketClients[fd];

	try {
		char buffer[4096];
		ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
		
		if (bytes == 0) { // EVAL : ici client deconnecte
			closeConnection(fd);
			return ;
		}
		
		if (bytes < 0) { // EVAL : ici erreur donc on close 
    		closeConnection(fd);
			earlyError(500, fd);
			return ;
		}
		
		client->appendBuffer(std::string(buffer, bytes));
		client->lastActivity = std::time(NULL);
		
		if (client->state == READING)
            client->parseRequest();
		
		if (client->state == BODY_READING)
            parseBody(client);
		
		if (client->state == READY) {
            client->state = PROCESSING;
            setPollOut(fd);
        }
	}
    catch (const ResponseException& e) {
        client->errorCode = e.getCode();
        client->state = ERROR;
        setPollOut(fd);
    }
}

void WebServer::parseBody(SocketClient* client){
	client->lastActivity = std::time(NULL);

	if (client->getRequest().getMethod() == "DELETE" || client->getRequest().getMethod() == "GET")
		client->ignoreBody = true;
    
	if (client->chunked){
        client->parsingChunked();
	}
    
	else if (client->contentLength){
        client->parsingContentLength();
	}

    else{
        client->parsingNoBody();
	}

    if (client->requestCompleted)
        client->state = READY;
}

void	WebServer::sendResponse(int fd){
	SocketClient* client = _socketClients[fd];
	
	try {
		HttpResponse res;
		const LocationConfig* location = findMatchingLocation(client);

		if (client->state != ERROR){
			try { resolvePath(client, location); }
			catch (const ResponseException& e) {
				client->state = ERROR;
				client->errorCode = e.getCode();
			}
		}
		
		if (client->state == ERROR) {
			res = buildErrorResponse(client->errorCode, client); 
		}
		
		else if (!location) {
			res = buildErrorResponse(404, client); }

		else if (location->returnCode) {
			res = buildRedirectResponse(location->returnCode, location->returnUrl, client); }
		else {
			std::string method = client->getRequest().getMethod();

			if (method == "GET")
				res = methodGet(client, location);
            else if (method == "POST")
                res = methodPost(client, location);
            else if (method == "DELETE")
                res = methodDelete(client, location);
		}

		std::string response = res.ResponseToString();

		size_t totalSent = 0;
		while (totalSent < response.size()) {
			ssize_t sent = send(fd, response.c_str() + totalSent, response.size() - totalSent, 0);
				
			if (sent <= 0 ) { // EVAL : erreur donc client supp
				closeConnection(fd);
				return;
			}
			totalSent += sent;
		}
		
		client->state = DONE;
		closeConnection(fd);
	}
	    catch (...) {
		earlyError(500, fd);
        closeConnection(fd);
    }
}