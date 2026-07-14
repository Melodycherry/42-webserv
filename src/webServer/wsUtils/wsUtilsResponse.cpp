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
#include "../utils/utilsGeneral.hpp"

const LocationConfig* WebServer::findMatchingLocation(const SocketClient* client) const {
    
    const ServerConfig* server = findMatchingConfig(client);
    if (!server)
        return NULL;

    const std::string& uri = client->getRequest().getUri();

    const LocationConfig* bestMatch = NULL;
    size_t bestLen = 0;
    for (size_t i = 0; i < server->locations.size(); ++i) {
        const LocationConfig& loc = server->locations[i];
        const std::string& locPath = loc.path;

        if (locPath.empty())
            continue;

        if (uri.compare(0, locPath.size(), locPath) != 0)
            continue;
        
        if (uri.size() > locPath.size() && !locPath.empty() && locPath[locPath.size() - 1] != '/' && uri[locPath.size()] != '/')
            continue;

        if (locPath.size() > bestLen) {
            bestMatch = &loc;
            bestLen = locPath.size();
        }
    }

    if (!bestMatch)
    {
        for (size_t i = 0; i < server->locations.size(); ++i) {
            if (server->locations[i].path == "/")
                bestMatch = &server->locations[i];
        }
    }

    return bestMatch;
}


const ServerConfig* WebServer::findMatchingConfig(const SocketClient* client) const {
    const std::vector<const ServerConfig*>& configs = client->getServer()->getServers();

    std::string host;
    const std::map<std::string, std::vector<std::string> >& headers = client->getRequest().getHeaders();

    std::map<std::string, std::vector<std::string> >::const_iterator it = headers.find("host");
    if (it != headers.end())
        host = it->second[0];
    
    std::string cleanHost = host.substr(0, host.find(':'));

    for (size_t i = 0; i < configs.size(); i++) {
        if (configs[i]->serverName == cleanHost)
            return configs[i];
    }

    if (!configs.empty())
        return configs[0];

    return NULL;
}