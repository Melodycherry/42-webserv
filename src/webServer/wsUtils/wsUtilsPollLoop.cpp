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

bool	WebServer::isServerFd(int fd){
	for (size_t i = 0; i < _socketServers.size(); ++i){
		if (_socketServers[i]->getFd() == fd)
			return true;
	}
	return false;
}

bool 	WebServer::isHeaderComplete(SocketClient* client){
	const std::string& buffer = client->getBuffer();
	size_t pos = buffer.find("\r\n\r\n");

	if (pos == std::string::npos)
		return false;
	return true;
}

void	WebServer::removePollFd(int fd){
	std::vector<struct pollfd>::iterator it;

	for (it = _pollFds.begin(); it != _pollFds.end(); ++it){
		if (it->fd == fd){
			_pollFds.erase(it);
			break;
		}
	}
}

void WebServer::closeConnection(int fd){ // voir avec chattos
	
	LOG("Closing connection fd = " << fd);

	if (_socketClients.find(fd) == _socketClients.end()) {
		return;
	}
	
	close(fd);

	std::map<int, SocketClient*>::iterator it = _socketClients.find(fd);
	if (it != _socketClients.end()) {
		delete it->second;
		_socketClients.erase(it);
	}

	removePollFd(fd);
}


void	WebServer::setPollOut(int fd){
	std::vector<struct pollfd>::iterator it;

	for (it = _pollFds.begin(); it != _pollFds.end(); ++it){
		if (it->fd == fd){
			it->events = POLLOUT;
			break;
		}
	}
}
