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

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"

#define DEFAULT_MAX_BODY_SIZE 5242880 // 5 MB

class ServerConfig {
public:
	std::string					port;
	std::string					host;
	std::string					serverName;
	std::string					root;
	std::vector<std::string>	index;

	size_t						maxBodySize;
	
	std::map<int, std::string>  errorPages;
	std::vector<LocationConfig> locations;

	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();
};

#endif

