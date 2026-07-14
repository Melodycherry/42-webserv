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

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <map>

class LocationConfig {
public:
	std::string				path;
	std::string				root;

	std::vector<std::string> methods;

	bool					autoindex;
	std::vector<std::string> index;

	int						returnCode;
	std::string				returnUrl;

	std::string				uploadStore;

	size_t					maxBodySize;

	std::map<std::string, std::string> cgiInfo;

	bool					cgiEnabled;

	bool					hasGet;
	bool					hasPost;
	bool					hasDelete;

	LocationConfig();
    LocationConfig(const LocationConfig& other);
    LocationConfig& operator=(const LocationConfig& other);
    ~LocationConfig();
};

#endif
