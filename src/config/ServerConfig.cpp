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

#include "ServerConfig.hpp"
#include "ParserConfig.hpp"

/* *************************************************** */
/*  Constructors, destructor, and assignment operator  */
/* *************************************************** */ 
ServerConfig::ServerConfig(){
	port = "8080";
	host = "0.0.0.0";
	serverName = "default_server";
	root = "./www";
	index.push_back("index.html");
	maxBodySize = DEFAULT_MAX_BODY_SIZE;
}
ServerConfig::ServerConfig(const ServerConfig &other) {
	port = other.port;
	host = other.host;
	serverName = other.serverName;
	root = other.root;
	index = other.index;
	maxBodySize = other.maxBodySize;
	errorPages = other.errorPages;
	locations = other.locations;
}
ServerConfig& ServerConfig::operator=(const ServerConfig &other) {
	if (this != &other) {
		port = other.port;
		host = other.host;
		serverName = other.serverName;
		root = other.root;
		index = other.index;
		maxBodySize = other.maxBodySize;
		errorPages = other.errorPages;
		locations = other.locations;
	}
	return *this;
}
ServerConfig::~ServerConfig() {}
