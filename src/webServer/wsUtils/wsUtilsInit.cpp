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

void	WebServer::initSockets(){
	std::vector<ServerConfig>::iterator it;

	for (it = _servers.begin(); it != _servers.end(); ++it){	
		
		bool exists = false;
		for (size_t i = 0; i < _socketServers.size(); ++i) {
			if (_socketServers[i]->getPort() == it->port) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			_socketServers.push_back(new SocketServer(it->port));
		}
	}
}

void	WebServer::cpyLinkConfig(){

	std::vector<ServerConfig>::iterator it;

	for (it = _servers.begin(); it != _servers.end(); ++it){	
		std::vector<SocketServer*>::iterator itserv;
		for (itserv = _socketServers.begin(); itserv != _socketServers.end(); ++itserv)
		{
			if ((*itserv)->getPort() == it->port){
				(*itserv)->addServer(&(*it));
				break;
			}
		}
	}	
}

void	WebServer::initPollStruct(){
	for (size_t i = 0; i < _socketServers.size(); ++i){
		int fd = _socketServers[i]->getFd();

		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		_pollFds.push_back(pfd);	
	}
}

void WebServer::checkTimeouts() {
    for (size_t i = 0; i < _pollFds.size(); ) {

        int fd = _pollFds[i].fd;

        if (!isServerFd(fd)) {
            std::map<int, SocketClient*>::iterator it = _socketClients.find(fd);
            if (it != _socketClients.end()) {

                SocketClient* client = it->second;
                if (client && isTimedOut(client)) {
					client->state = ERROR;
					client->errorCode = 400;
					earlyError(400, fd);
					closeConnection(fd);
                }
            }
        }
        i++;
    }
}

bool WebServer::isTimedOut(const SocketClient* client) const
{
    const int TIMEOUT = 5;

    time_t now = time(NULL);
    double diff = difftime(now, client->lastActivity);

    LOG ("TIMEOUT CHECK fd =" << client->getFd() << " now=" << now << " last=" << client->lastActivity << " diff=" << diff);

    return diff >= TIMEOUT;
}
