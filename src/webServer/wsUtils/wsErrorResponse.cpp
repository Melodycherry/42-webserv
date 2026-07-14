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

std::string loadFile(const std::string& path){
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return "";

    std::ostringstream ss;
    ss << file.rdbuf();

    return ss.str();
}

std::string resolveErrorPage(int code, const ServerConfig* config)
{
    if (config) {
        std::map<int, std::string>::const_iterator it = config->errorPages.find(code);
        if (it != config->errorPages.end())
            return it->second;
    }

    static std::map<int, std::string> defaults;

    if (defaults.empty())
    {
        defaults[400] = "www/errorPages/400.html";
        defaults[403] = "www/errorPages/403.html";
        defaults[404] = "www/errorPages/404.html";
        defaults[405] = "www/errorPages/405.html";
        defaults[413] = "www/errorPages/413.html";
        defaults[414] = "www/errorPages/414.html";
        defaults[415] = "www/errorPages/415.html";
        defaults[431] = "www/errorPages/431.html";
        defaults[500] = "www/errorPages/500.html";
        defaults[503] = "www/errorPages/503.html";
        defaults[504] = "www/errorPages/504.html";
    }

    std::string ret;

    std::map<int, std::string>::const_iterator it = defaults.find(code);
    if (it != defaults.end())
        ret = it->second;
    else
        ret = "/errorPages/generic.html";
    return ret;
}

HttpResponse WebServer::buildErrorResponse(int code, const SocketClient* client)
{
    HttpResponse res;
    std::string statusText;
    std::string connection;

    const ServerConfig* config = findMatchingConfig(client);
    const std::map<int, std::string> errorPages = config->errorPages;

    std::string filePath = resolveErrorPage(code, config);
    std::string body = loadFile(filePath);

    if (body.empty()) {
        body =
            "<html>"
            "<body style='text-align:center;font-family:sans-serif;'>"
            "<h1>Error " + longToString(code) + "</h1>"
            "<p>Unable to load error page.</p>"
            "</body>"
            "</html>";
    }

    if (code == 400) {
        statusText = "Bad Request";
        connection = "close";
    }
    else if (code == 403) {
        statusText = "Forbidden";
        connection = "keep-alive";
    }
    else if (code == 404) {
        statusText = "Not Found";
        connection = "keep-alive";
    }
	else if (code == 405) {
		statusText = "Method Not Allowed";
        connection = "keep-alive";
	}
    else if (code == 413) {
        statusText = "Payload Too Large";
        connection = "close";
    }
    else if (code == 414) {
        statusText = "URI Too Long";
        connection = "close";
    }
    else if (code == 415) {
        statusText = "Unsupported Media Type";
        connection = "keep-alive";
    }
    else if (code == 431) {
        statusText = "Request Header Fields Too Large";
        connection = "close";
    }
    else if (code == 500) {
        statusText = "Internal Server Error";
        connection = "keep-alive";
    }
    else {
        statusText = "Error";
        connection = "close";
    }

    res.statusLine = "HTTP/1.1 " + longToString(code) + " " + statusText;
    res.body = body;

    res.headers["Content-Length"].push_back(longToString(res.body.size()));
    res.headers["Content-Type"].push_back("text/html");

    if (client && client->keepAlive)
        res.headers["Connection"].push_back(connection);
    else
        res.headers["Connection"].push_back("close");

    return res;
}


void WebServer::earlyError(int code, int fd)
{
    if (fd > 0) {
        HttpResponse res;
        std::string statusText;
        std::string connection;
        std::string filePath = resolveErrorPage(code, NULL);
        std::string body = loadFile(filePath);

        if (body.empty()) {
            body =
                "<html>"
                "<body style='text-align:center;font-family:sans-serif;'>"
                "<h1>Error " + longToString(code) + "</h1>"
                "<p>Unable to load error page.</p>"
                "</body>"
                "</html>";
        }

        res.statusLine = "HTTP/1.1 " + longToString(code) + " " + statusText;
        res.body = body;

        res.headers["Content-Length"].push_back(longToString(res.body.size()));
        res.headers["Content-Type"].push_back("text/html");
        res.headers["Connection"].push_back("close");

        std::string ret = res.ResponseToString();
        
        size_t totalSent = 0;
		while (totalSent < ret.size()) {
            ssize_t sent = send(fd, ret.c_str(), ret.size(), 0);
            if (sent <= 0 ) { // EVAL : erreur donc client supp
				closeConnection(fd);
				return;
			}
			totalSent += sent;
        }
        close(fd);
    }

}