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

#ifndef PARSERCONFIG_HPP
#define PARSERCONFIG_HPP

#include "Exceptions.hpp"

class ServerConfig; 
class LocationConfig;

class ParserConfig {

	private:
		std::vector<ServerConfig> 	_servers;
		std::vector<std::string> 	_tokens;

		// CLEAN ET TOKEN
		std::string 				readFile(const std::string &path);
		void 						removeComments(std::string &content);
		void					 	tokenize(const std::string &content);

		// PARSE SERVEUR ET LOCATION 
		void 						parseServer(std::vector<std::string>::iterator &it);
		void 						parseLocation(std::vector<std::string>::iterator &it, ServerConfig &server);

		// HANDLERS serveur
		void						handleListen(std::vector<std::string>::iterator &it, ServerConfig &server);
		void 						handleHost(std::vector<std::string>::iterator &it, ServerConfig &server);
		void						handleServerName(std::vector<std::string>::iterator &it, ServerConfig &server);
		void						handleErrorPage(std::vector<std::string>::iterator &it, ServerConfig &server);
		void						handleMaxBodySize(std::vector<std::string>::iterator &it, ServerConfig &server);
		void						handleRoot(std::vector<std::string>::iterator &it, ServerConfig &server);
		void						handleIndex(std::vector<std::string>::iterator &it, ServerConfig &server);

		// HANDLERS location
		void						handleMethods(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleAutoindex(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleMaxBodySize(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleRoot(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleIndex(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleReturn(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleUploadStore(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleCgi(std::vector<std::string>::iterator &it, LocationConfig &location);
		void						handleCgiEnabled(std::vector<std::string>::iterator &it, LocationConfig &location);

		// CHECK					
		bool						validateValue(std::vector<std::string>::iterator &it);
		void						checkSemicolon(std::vector<std::string>::iterator &it);
		void						checkBracketsBalance(const std::string &content);

		// VERIF
		void 						verifyConfig();

	public:
		ParserConfig();
		ParserConfig(const ParserConfig &other);
		ParserConfig &operator=(const ParserConfig &other);
		~ParserConfig();

		void parse(const std::string &configFilePath);
    	const std::vector<ServerConfig> &getServers() const { return _servers; }

};


#endif
