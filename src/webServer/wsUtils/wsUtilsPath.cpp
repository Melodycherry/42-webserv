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

void WebServer::resolvePath(SocketClient* client, const LocationConfig* location) {
    const std::string& uri = client->getRequest().getUri();
    int fd = client->getFd();
    
    size_t pos = uri.find('?');
    
    std::string locationPath = location->path;

    std::string path;
    std::string queryStr;

    if (pos == std::string::npos) {
        path = uri;
        queryStr = "";
		client->getRequest().setOriginPath(uri);
		client->getRequest().setOriginQuery(queryStr);

    } else {
        path = uri.substr(0, pos);
        queryStr = uri.substr(pos + 1);
		client->getRequest().setOriginPath(uri);
		client->getRequest().setOriginQuery("?" + queryStr);
    }

    resolveQuery(client, queryStr);
	std::string finalPath;
    finalPath = decodePath(path, fd);
    checkPathSecurity(finalPath, fd);
    finalPath = normalizePath(finalPath, fd);
    checkErrorPath(finalPath, fd);
    
    std::string root = findRoot(client);
    cleanFinalPath(root, finalPath);
    if ((locationPath != "/") && (finalPath.compare(0, locationPath.size(), locationPath) != 0)) {
        throw ResponseException(fd, 403);
    }
    
    finalPath = root + finalPath;

    client->getRequest().setPath(finalPath);
}

std::string	WebServer::decodePath(const std::string &path, int fd){
	std::string output;

	if (path.empty()){
		throw ResponseException(fd, 400);
    }

	for (size_t i = 0 ; i < path.size() ; i++){
		if (path[i] == '%') {
			if (i + 2 >= path.size())
                throw ResponseException(fd, 400);
            
			char c1 = path[i + 1];
			char c2 = path[i + 2];

			if (c1 == '0' && c2 == '0')
                throw ResponseException(fd, 400);
            
			if (!isHex(c1) || !(isHex(c2)))
				throw ResponseException(fd, 400);

			char decoded = hexToChar(c1, c2);

			output += decoded;
			i += 2;
		}

		else 
			output += path[i];
	}

	if (output.find("%2f") != std::string::npos){
        throw ResponseException(fd, 403);
    }
	
	if (output.find('\0') != std::string::npos){
        throw ResponseException(fd, 403);
    }
	
	return output;
}

std::string	WebServer::normalizePath(const std::string &path, int fd){
	std::vector<std::string> segmentPath;
	std::stringstream ss(path);
	std::string segment;

	while((getline(ss, segment, '/'))){
		if (segment == "" || segment == ".")
			continue;
		
		else if (segment == ".."){
			if (segmentPath.empty())
				throw ResponseException(fd, 400);
			segmentPath.pop_back();
		}

		else 
			segmentPath.push_back(segment);
	}

	std::string result = "/";
	for (size_t i = 0 ; i < segmentPath.size() ; i++){
		result += segmentPath[i];
		if (i != segmentPath.size() - 1)
			result += "/";
	}

	return result;
}

void WebServer::cleanFinalPath(std::string& root, std::string& finalPath) {
    if (root.empty())
        return;

    if (finalPath.empty()) {
        finalPath = "/";
        return;
    }

    if (root[root.size() - 1] == '/' && finalPath[0] == '/')
        finalPath = finalPath.substr(1);

    else if (root[root.size() - 1] != '/' && finalPath[0] != '/')
        root += "/";
}

std::string WebServer::findRoot(const SocketClient* client){
	const std::vector<const ServerConfig*>& conf = client->getServer()->getServers();
    const HttpRequest& req = client->getRequest();
    const std::map<std::string, std::vector<std::string> > & headers = req.getHeaders();

    std::string host;
    std::map<std::string, std::vector<std::string> >::const_iterator it = headers.find("host");
    if (it != headers.end())
        host = it->second[0];

    std::string recupRoot;
    for (size_t i = 0; i < conf.size(); ++i) {
        const ServerConfig* cfg = conf[i];
        if (cfg->serverName == host) {
            recupRoot = cfg->root;
            break;
        }
    }
    if (recupRoot.empty() && !conf.empty()){
        recupRoot = conf[0]->root;
    }
    
	return recupRoot;
}

void	WebServer::checkErrorPath(const std::string &path, int fd){
	if (path.find("%") != std::string::npos)
		throw ResponseException(fd, 403);

	if (path.find("//") != std::string::npos)
		throw ResponseException(fd, 403);

	for (size_t i = 0 ; i < path.size() ; i++){ 
		if (path[i] < ' ' || path[i] > '~')
			throw ResponseException(fd, 403);
	}
}


void WebServer::resolveQuery(SocketClient* client, std::string queryStr) {
    std::map<std::string, std::vector<std::string> > query;

    size_t posStart = 0;

    while (posStart < queryStr.length()) {

        size_t posEnd = queryStr.find('&', posStart);
        if (posEnd == std::string::npos)
            posEnd = queryStr.length();

        std::string chunk = queryStr.substr(posStart, posEnd - posStart);

        size_t equalPos = chunk.find('=');

        std::string key;
        std::string value;

        if (equalPos == std::string::npos) {
            key = chunk;
            value = "";
        } else {
            key = chunk.substr(0, equalPos);
            value = chunk.substr(equalPos + 1);
        }

        key = UrlDecode(client, key);
        value = UrlDecode(client, value);

        if (!key.empty())
            query[key].push_back(value);

        posStart = posEnd + 1;
    }

    client->getRequest().setQuery(query);
}


std::string WebServer::UrlDecode(const SocketClient *client, std::string entry) {
    std::string result;

    for (size_t i = 0; i < entry.length(); i++) {

        if (entry[i] == '%') {
            if (i + 2 >= entry.size())
                throw ResponseException(client->getFd(), 400);

            char c1 = entry[i + 1];
            char c2 = entry[i + 2];

            if (!isHex(c1) || !isHex(c2))
                throw ResponseException(client->getFd(), 400);

            char decoded = hexToChar(c1, c2);

            if (decoded == '\0')
                throw ResponseException(client->getFd(), 400);

            result += decoded;
            i += 2;
        }
        else if (entry[i] == '+') 
            result += ' ';
        else 
            result += entry[i];
        
    }
    return result;
}



void WebServer::checkPathSecurity(const std::string &uri, int fd)
{
    if (uri.empty())
        throw ResponseException(fd, 400);

    if (uri[0] != '/')
        throw ResponseException(fd, 400);

    int balance = 0;
    size_t pos = 0;

    while (pos < uri.size())
    {
        while (pos < uri.size() && uri[pos] == '/')
            pos++;

        if (pos >= uri.size())
            break;

        size_t end = uri.find('/', pos);
        if (end == std::string::npos)
            end = uri.size();

        std::string part = uri.substr(pos, end - pos);

        if (part == "."){ }
        else if (part == "..") {
            balance--;
            if (balance < 0)
                throw ResponseException(fd, 403);
        }
        else {
            balance++;
        }
        pos = end;
    }
}


bool WebServer::isMethodAllowed(const std::string& path, const std::string& method, const LocationConfig* location)
{
    (void)path;
    if (!location)
        return false;

    if (path.compare(0, location->path.size(), location->path) != 0)
        return false;
    
    if (method == "GET")
        return location->hasGet;
    
    if (method == "POST")
        return location->hasPost;
    
    if (method == "DELETE")
        return location->hasDelete;
    
    return false;
}
