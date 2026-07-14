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


/* ************************************************** */
/* Directory					                      */
/* ************************************************** */

HttpResponse WebServer::handleDirectory(const SocketClient* client, const std::string& path){
	const ServerConfig* config = findMatchingConfig(client);
	const LocationConfig* loc = findMatchingLocation(client);

	const std::vector<std::string>* indexes;
	bool autoindex;
	std::string root = config->root;

	if (loc && !loc->index.empty())
		indexes = &loc->index;
	else
		indexes = &config->index;

	if (loc)
		autoindex = loc->autoindex;
	else
		autoindex = false;

    if (!indexes->empty()) {
        for (size_t i = 0; i < indexes->size(); i++) {
            std::string fullPath = path;

            if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
                fullPath += "/";

            fullPath += (*indexes)[i];

            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
                if (access(fullPath.c_str(), R_OK) == 0)
                    return serveFile(client, fullPath);
            }
        }
    }
    if (autoindex) 
        return generateListing(path, client);

    return buildErrorResponse(403, client);
}


std::string WebServer::getMimeType(std::string path){
	size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream";

    std::string ext = path.substr(dot);

	if (ext == ".html" || ext == ".htm"){
        return "text/html";}

    if (ext == ".css"){
        return "text/css";}

    if (ext == ".js"){
        return "application/javascript";}

    if (ext == ".json"){
        return "application/json";}

    if (ext == ".png"){
        return "image/png";}

    if (ext == ".jpg" || ext == ".jpeg"){
        return "image/jpeg";}

    if (ext == ".gif"){
        return "image/gif";}

    if (ext == ".txt"){
        return "text/plain";}

    if (ext == ".pdf"){
        return "application/pdf";}

	return "application/octet-stream"; // standard par defaut
}


HttpResponse WebServer::serveFile(const SocketClient* client, const std::string& path){

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) {
        return buildErrorResponse(500, client);
    }
	
	std::ostringstream ss;
	ss << file.rdbuf();
	std::string body = ss.str();
	HttpResponse res = fillResponseOK(body, body.size(), getMimeType(path), client);
	return res;
}

HttpResponse	WebServer::generateListing(const std::string &path, const SocketClient* client){
	DIR* dir = opendir(path.c_str());
	if (!dir)
		return buildErrorResponse(403, client);

	std::stringstream body;

	body << "<html><head><title>Index of " << path << "</title></head><body>";
	body << "<h1>Index of " << path << "</h1><hr><ul>";

	struct dirent* entry;

	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;

		body << "<li><a href=\"" << name;
        if (entry->d_type == DT_DIR)
            body << "/";
        body << "\">" << name << "</a></li>";
	}
	
	body << "</ul><hr></body></html>";

	closedir(dir);

	std::string content = body.str();
	return fillResponseOK(content, content.size(), "text/html", client);
}

HttpResponse	WebServer::fillResponseOK(std::string body, long size, std::string type, const SocketClient* client){
	HttpResponse res;
	
	res.statusLine = "HTTP/1.1 200 OK";
	res.body = body;

	std::string sizeStr = longToString(size);

	res.headers["Content-Length"].push_back(sizeStr);
	res.headers["Content-Type"].push_back(type);
	if (client && client->keepAlive)
        res.headers["Connection"].push_back("keep-alive");
	else 
    	res.headers["Connection"].push_back("close");
	return res;
}
