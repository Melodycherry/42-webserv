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

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <poll.h>
#include <map>
#include <sys/stat.h>
#include <fstream>
#include <time.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <dirent.h>


#include "SocketClient.hpp"
#include "SocketServer.hpp"
#include "ServerConfig.hpp"
#include "Exceptions.hpp"
#include "../http/HttpResponse.hpp"
#include "../utils/utilsGeneral.hpp"



#define DEBUG 1

#if DEBUG
    #define LOG(x) std::cout << x << std::endl;
#else
    #define LOG(x)
#endif




#define MAX_REQUEST_SIZE 8192
#define MAX_URI_SIZE 2048
class WebServer{

	private :
		
		std::vector<SocketServer*>	    _socketServers;
		std::map<int, SocketClient*>	_socketClients;
		std::vector<ServerConfig>	    _servers;
		std::vector<struct pollfd>	    _pollFds;
		
		WebServer(const WebServer &other);
		WebServer& operator=(const WebServer &other);
		
	public :

		WebServer(const std::vector<ServerConfig> &servers);
		~WebServer();
	
		bool	_running;
		
		//PAUL LOOP
		void	pollLoop();
		void	acceptClient(int fd);
		void	handleRequest(int fd);
		void	sendResponse(int fd);
		

		void	parseBody(SocketClient* client);


		//INIT -> file : wsUntilsInit.cpp
		void	cpyLinkConfig();
		void	initSockets();
		void	initPollStruct();
		void 	checkTimeouts();
		bool	isTimedOut(const SocketClient* client) const;

		//UTILS RESPONSE -> file : wsResponse.cpp
		HttpResponse	methodGet(SocketClient* client, const LocationConfig* location);
		HttpResponse	methodPost(SocketClient* client, const LocationConfig* location);
		HttpResponse	methodDelete(SocketClient* client, const LocationConfig* location);

		//UTILS GET -> file :  wsUtilsGet.cpp
		HttpResponse	handleDirectory(const SocketClient* client, const std::string& path);
		HttpResponse	generateListing(const std::string &path, const SocketClient* client);
		HttpResponse	serveFile(const SocketClient* client, const std::string& path);
		HttpResponse	fillResponseOK(std::string body, long size, std::string type, const SocketClient* client);
		
		//UTILS POST -> file : wsUtilsPost.cpp
		bool			isCGI(const LocationConfig* location, const std::string& path);
		HttpResponse	executeCGI(const SocketClient* client, const std::string& path);
		HttpResponse	executeStatic(const SocketClient* client, const std::string& path);
		HttpResponse	createCGIResponse(const SocketClient* client, std::string output);
		std::string		extractPathInfo(const std::string& uri, const std::string& scriptPath);

		std::map<std::string, std::vector<std::string> > createEnvp(const SocketClient* client, const std::string& path);
		char**			convertMapToChar(const std::map<std::string, std::vector<std::string> >& env);
		// 
		const ServerConfig*		findMatchingConfig(const SocketClient* client) const;
		const LocationConfig*	findMatchingLocation(const SocketClient* client) const;
		HttpResponse			buildRedirectResponse(int returnCode, const std::string& url, SocketClient* client);
		bool					fullDelete(const std::string& path);
		
		//PATH -> file : wsPath.cpp
		void		resolvePath(SocketClient* client, const LocationConfig* location);
		void		checkPathSecurity(const std::string &path, int fd);
		std::string	decodePath(const std::string &path, int fd);
		std::string	normalizePath(const std::string &path, int fd);
		void		checkErrorPath(const std::string &path, int fd);
		std::string findRoot(const SocketClient* client);
		void		cleanFinalPath(std::string& root, std::string& finalPath);
		void		resolveQuery(SocketClient* client, std::string path);
		std::string UrlDecode(const SocketClient *client, std::string entry);
		bool		isMethodAllowed(const std::string& path, const std::string& method, const LocationConfig* location);

		std::string getMimeType(std::string path);

		
		//PAUL LOOP UTILS -> file : wsUtilsPollLoop.cpp
		bool	isServerFd(int fd);
		bool 	isHeaderComplete(SocketClient* client);
		
		void	closeConnection(int fd);
		void	removePollFd(int fd);
		void	setPollOut(int fd);

		HttpResponse buildErrorResponse(int code, const SocketClient* client);
		void	 earlyError(int code, int fd);
};

#endif









































	// 	////////// TTTEEEEEEEESTTTTT

	// 	void testPrintSocket() const {
	// std::cout << "=== WebServer SocketServers ===" << std::endl;

	// int socketCount = 0;

	// for (std::vector<SocketServer*>::const_iterator sockIt = _socketServers.begin();
	//      sockIt != _socketServers.end(); ++sockIt) {
	//     const SocketServer* sock = *sockIt;

	//     std::cout << "\n[" << ++socketCount << "] SocketServer" << std::endl;
	//     std::cout << "  Port: " << sock->getPort() << std::endl;
	//     std::cout << "  Socket FD: " << sock->getFd() << std::endl;

	//     const std::vector<const ServerConfig*>& servers = sock->getServers();
	//     if (servers.empty()) {
	//         std::cout << "  No ServerConfig attached!" << std::endl;
	//         continue;
	//     }

	//     int serverCount = 0;
	//     for (std::vector<const ServerConfig*>::const_iterator servIt = servers.begin();
	//          servIt != servers.end(); ++servIt) {
	//         const ServerConfig* server = *servIt;

	//         std::cout << "  [" << ++serverCount << "] ServerConfig" << std::endl;
	//         std::cout << "    MaxBodySize: " << server->maxBodySize << std::endl;

	//         const std::vector<std::string>& index = server->index;
	//         std::cout << "    Index files:";
	//         if (index.empty()) std::cout << " (none)" << std::endl;
	//         else {
	//             std::cout << std::endl;
	//             for (std::vector<std::string>::const_iterator idxIt = index.begin();
	//                  idxIt != index.end(); ++idxIt) {
	//                 std::cout << "      - " << *idxIt << std::endl;
	//             }
	//         }

	//         const std::map<int,std::string>& errors = server->errorPages;
	//         std::cout << "    Error pages:";
	//         if (errors.empty()) std::cout << " (none)" << std::endl;
	//         else {
	//             std::cout << std::endl;
	//             for (std::map<int,std::string>::const_iterator errIt = errors.begin();
	//                  errIt != errors.end(); ++errIt) {
	//                 std::cout << "      " << errIt->first << " -> " << errIt->second << std::endl;
	//             }
	//         }

	//         const std::vector<LocationConfig>& locations = server->locations;
	//         std::cout << "    Locations:";
	//         if (locations.empty()) std::cout << " (none)" << std::endl;
	//         else {
	//             std::cout << std::endl;
	//             int locCount = 0;
	//             for (std::vector<LocationConfig>::const_iterator locIt = locations.begin();
	//                  locIt != locations.end(); ++locIt) {
	//                 const LocationConfig& loc = *locIt;
	//                 std::cout << "      [" << ++locCount << "] Location" << std::endl;
	//                 std::cout << "        Path: " << loc.path << std::endl;
	//                 std::cout << "        Root: " << loc.root << std::endl;

	//                 const std::vector<std::string>& locIndex = loc.index;
	//                 std::cout << "        Index files:";
	//                 if (locIndex.empty()) std::cout << " (none)" << std::endl;
	//                 else {
	//                     std::cout << std::endl;
	//                     for (std::vector<std::string>::const_iterator idxIt2 = locIndex.begin();
	//                          idxIt2 != locIndex.end(); ++idxIt2) {
	//                         std::cout << "          - " << *idxIt2 << std::endl;
	//                     }
	//                 }

	//                 const std::vector<std::string>& methods = loc.methods;
	//                 std::cout << "        Allowed Methods:";
	//                 if (methods.empty()) std::cout << " (none)" << std::endl;
	//                 else {
	//                     std::cout << " ";
	//                     for (std::vector<std::string>::const_iterator mIt = methods.begin();
	//                          mIt != methods.end(); ++mIt) {
	//                         std::cout << *mIt << " ";
	//                     }
	//                     std::cout << std::endl;
	//                 }
	//             }
	//         }

	//     } // end ServerConfig loop
	// } // end SocketServer loop

	// std::cout << "\n=== End of WebServer Info ===" << std::endl;
	//}