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


/* ************************************************** */
/* UTILS RESPONSE.     		                          */
/* ************************************************** */

HttpResponse	WebServer::methodGet(SocketClient* client, const LocationConfig* location){
    std::string path = client->getRequest().getPath();

	if (!isMethodAllowed(client->getRequest().getOriginPath(), "GET", location)){
        return buildErrorResponse(405, client);
	}
    if (isCGI(location, path)) {
        return executeCGI(client, path);
	}
    
	struct stat st;
	if (stat(path.c_str(), &st) < 0){
			return buildErrorResponse(404, client);
	}
	if (access(path.c_str(), R_OK) != 0){ 
			return buildErrorResponse(403, client);
	}
	if (st.st_mode & S_IFDIR){
			return handleDirectory(client, path);
	}

	return serveFile(client, path);
}

HttpResponse	WebServer::methodPost(SocketClient* client, const LocationConfig* location){
	std::string path = client->getRequest().getPath();

	if (isCGI(location, path))
		return (executeCGI(client, path));
    if (!isMethodAllowed(client->getRequest().getOriginPath(), "POST", location))
        return buildErrorResponse(405, client);
	else 
		return (executeStatic(client, path));
}

HttpResponse WebServer::methodDelete(SocketClient* client, const LocationConfig* location){
	std::string path = client->getRequest().getPath();

	if (isCGI(location, path))
    	return executeCGI(client, path);

	else {
		
		if (!isMethodAllowed(client->getRequest().getOriginPath(), "DELETE", location)){
        	return buildErrorResponse(405, client);
		}

		std::string dir = path.substr(0, path.find_last_of('/'));

		struct stat st;
		if (stat(path.c_str(), &st) < 0)
			return buildErrorResponse(404, client);

		if (!fullDelete(path)) {
			return buildErrorResponse(500, client);
		}

		HttpResponse res;
		res.statusLine = "HTTP/1.1 204 No Content";
		return res;
	}
}

bool WebServer::fullDelete(const std::string& path) {
    struct stat st;

    if (stat(path.c_str(), &st) != 0)
        return false;

    if (S_ISDIR(st.st_mode)) {
        DIR* dir = opendir(path.c_str());
        if (!dir) return false;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;

            if (name == "." || name == "..") continue;

            std::string fullPath = path;
            if (path[path.length() - 1] != '/') {
                fullPath += "/";
            }
            fullPath += name;

            if (!fullDelete(fullPath)) {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
        return (std::remove(path.c_str()) == 0);
    } 
    
    else 
        return (std::remove(path.c_str()) == 0);
}

HttpResponse WebServer::buildRedirectResponse(int returnCode, const std::string& url, SocketClient* client){
	HttpResponse res;

	std::string status;
	std::string message;

	if (returnCode == 301){
		status = "Moved Permanently";
		message = "Moved Permanently";
	}
	else if (returnCode == 302){
		status = "Found";
		message = "Found";
	}
	else if (returnCode == 307){
		status = "Temporary Redirect";
		message = "Temporary Redirect";
	}
	else if (returnCode == 308){
		status = "Permament Redirect";
		message = "Permament Redirect";
	}
	else status = "Redirect";

    res.body =
        "<html>"
        "<head><title>" + longToString(returnCode) + " " + status + "</title></head>"
        "<body style='text-align:center;font-family:sans-serif;'>"
        "<h1>" + longToString(returnCode) + " - " + status + "</h1>"
        "<p>" + message + "</p>"
        "</body>"
        "</html>";

	res.statusLine = "HTTP/1.1 " + longToString((long)returnCode) + " " + status; 
	res.headers["location"].push_back(url);
	res.headers["content-length"].push_back("0");

	if (client->keepAlive)
		res.headers["connection"].push_back("keep-alive");
	else
		res.headers["connection"].push_back("close");

	return res;
}
