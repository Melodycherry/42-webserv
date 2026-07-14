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

	#include "SocketServer.hpp"
	#include "Exceptions.hpp"
	#include <iostream>
	#include <cstring>
	
/* ************************************************** */
/* construtor & destructors                           */
/* ************************************************** */

SocketServer::SocketServer() : _sockFd(-1), _res(NULL), _port("default"){
	createSocket();
}
SocketServer::SocketServer(std::string port) : _sockFd(-1), _res(NULL), _port(port){
	createSocket();
}
SocketServer::~SocketServer(){
	if (_sockFd != -1)
		close(_sockFd);
	if (_res)
		freeaddrinfo(_res);
}

/* ************************************************** */
/* getters & setters.                                 */
/* ************************************************** */

int									SocketServer::getFd() const{ return (_sockFd); }
const std::string&					SocketServer::getPort() const{ return (_port); }
const std::vector< const ServerConfig*>&	SocketServer::getServers() const{ return (_servers); }
struct addrinfo* 					SocketServer::getAddrinfo() const { return _res;}

void 								SocketServer::addServer(ServerConfig* server){ _servers.push_back(server); }


/* ************************************************** */
/* socket creation                                    */
/* ************************************************** */

static bool setSocket(int sockFd);

void    SocketServer::createSocket(){
	struct addrinfo hints;

	if (initStructGetaddrinfo(hints, &_res) != 0)
		throw SocketException("Error : socket not created");

	_sockFd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
	if (_sockFd == -1)
		throw SocketException("Error : socket not created");
	
	if (!setSocket(_sockFd))
		throw SocketException("Error : socket settings failed");

	if (bind(_sockFd, _res->ai_addr, _res->ai_addrlen) == -1)
		throw SocketException("Error : bind has failed");
	
	if (listen(_sockFd, SOMAXCONN) == -1)
		throw SocketException("Error : listen has failed");
}

static bool setSocket(int sockFd)
{
	int opt = 1;
	if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return false;

	int flags = fcntl(sockFd, F_GETFL, 0);
	if (flags < 0)
		return false;
	if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) < 0)
		return false;

	int fdFlags = fcntl(sockFd, F_GETFD);
	if (fdFlags < 0)
		return false;
	if (fcntl(sockFd, F_SETFD, fdFlags | FD_CLOEXEC) < 0)
		return false;

	return true;
}

int SocketServer::initStructGetaddrinfo(struct addrinfo& hints, struct addrinfo** res){
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int ret = getaddrinfo(NULL, _port.c_str(), &hints, res);
	
	return ret;
}

/* ************************************************** */
/* other socket fucntions                             */
/* ************************************************** */

int     SocketServer::acceptClient(){
	return 1;
}

void    SocketServer::closeSocket(){
	
}
