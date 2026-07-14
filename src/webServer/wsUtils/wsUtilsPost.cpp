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
/* Static						                      */
/* ************************************************** */

std::string getDirectoryFromPath(const std::string& path)
{
    if (path.empty())
        return "";

    size_t pos = path.find_last_of('/');

    if (pos == std::string::npos)
        return "";

    return path.substr(0, pos + 1);
}

HttpResponse	WebServer::executeStatic(const SocketClient* client, const std::string& path) {
    if (path.find("/cgi-bin/") != std::string::npos){
        return buildErrorResponse(403, client);
    }
   
    const HttpRequest& req = client->getRequest();

    if (req.getBodyPath().empty())
        return buildErrorResponse(400, client);

    std::ifstream src(req.getBodyPath().c_str(), std::ios::binary);
    if (!src)
        return buildErrorResponse(500, client);

    std::string dir = getDirectoryFromPath(path);
    struct stat st;
    if (stat(dir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)){
        return buildErrorResponse(404, client);
    }

    if (access(dir.c_str(), W_OK) != 0) {
        return buildErrorResponse(403, client);
    }

    std::ofstream dst(path.c_str(), std::ios::binary | std::ios::trunc);

    if (!dst.is_open()) {
        return buildErrorResponse(500, client);
    }

    dst << src.rdbuf();

    HttpResponse res;
    res.statusLine = "HTTP/1.1 201 OK";
    res.headers["Content-Type"].push_back("text/plain");
    res.body = "File uploaded\n";
    return res;
}

/* ************************************************** */
/* CGI  						                      */
/* ************************************************** */

bool WebServer::isCGI(const LocationConfig* location, const std::string& path){
    if (!location){
        return false;
    }
    
    if (location->cgiInfo.empty()){
        return false;
    }

    size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos){
        return false;
    }

    std::string ext = path.substr(lastDot);
    if (location->cgiInfo.find(ext) != location->cgiInfo.end()){
        return true;
    }

    return false;
}

HttpResponse	WebServer::executeCGI(const SocketClient* client, const std::string& path) {
    if (access(path.c_str(), F_OK) == -1)
        return buildErrorResponse(404, client);

    if (access(path.c_str(), X_OK) == -1)
        return buildErrorResponse(403, client);

    int bodyFd = -1;

    std::string bodyPath = client->getRequest().getBodyPath();
    if (!bodyPath.empty()) {
        bodyFd = open(bodyPath.c_str(), O_RDONLY);
        if (bodyFd == -1)
            return buildErrorResponse(500, client);
    }

    std::map<std::string, std::vector<std::string> > map = createEnvp(client, path);
    char** envp = convertMapToChar(map);
    if (envp == NULL) {
        return buildErrorResponse(500, client);
    }

    int pipeFd[2] = {-1};
    
    if (pipe(pipeFd) == -1) {
        safeClose(&bodyFd);
        freeTab(&envp);
        return buildErrorResponse(500, client);
    }

    pid_t pid = fork();
    if (pid == -1) {
        safeClose(&bodyFd);
        safeClose(&pipeFd[0]); safeClose(&pipeFd[1]);
        freeTab(&envp);
        return buildErrorResponse(500, client);
    }

    if (pid == 0) {
        if (bodyFd != -1)
            dup2(bodyFd, STDIN_FILENO);
        else
            close(STDIN_FILENO);
        dup2(pipeFd[1], STDOUT_FILENO);

        safeClose(&bodyFd);
        safeClose(&pipeFd[0]); safeClose(&pipeFd[1]);

        char* argv[] = { const_cast<char*>(path.c_str()), NULL };
        execve(path.c_str(), argv, envp);
        
        std::exit(1);
    }

    safeClose(&bodyFd);
    safeClose(&pipeFd[1]);
    
    int status;
    time_t start = time(NULL);
    const int MAX_WAIT = 5;

    while (true) {
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid)
            break;

        if (result == -1) {
            kill(pid, SIGKILL);
            return buildErrorResponse(500, client);
        }

        if (difftime(time(NULL), start) > MAX_WAIT) {
            kill(pid, SIGKILL);
            safeClose(&pipeFd[0]);
            freeTab(&envp);
            return buildErrorResponse(504, client);
        }
        usleep(1000);
    }
    
    std::string output;
    char buffer[4096];
    ssize_t bytes;

    while ((bytes = read(pipeFd[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytes);
    
    safeClose(&pipeFd[0]);
    freeTab(&envp);

    if (output.empty())
        return buildErrorResponse(500, client);
    
    return createCGIResponse(client, output);
}

HttpResponse	WebServer::createCGIResponse(const SocketClient* client, std::string raw){
    HttpResponse res;

    if (raw.empty())
        return buildErrorResponse(500, client);

    size_t pos = raw.find("\r\n\r\n");
    size_t sep_len = 4;

    if (pos == std::string::npos)
        pos = raw.find("\n\n");

    if (pos == std::string::npos) {
        res.statusLine = "HTTP/1.1 200 OK";
        res.body = raw;
        return res;
    }

    if (client->getRequest().getMethod() == "POST") {
        if (pos == std::string::npos){
            return buildErrorResponse(500, client);
        }
    }
    
    std::string headers = raw.substr(0, pos);
    std::string body = raw.substr(pos + sep_len);

    std::vector<std::string> lines = splitLines(headers);

    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].empty())
            continue;

        size_t colon = lines[i].find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = trim(lines[i].substr(0, colon));
        key = toLower(key);
        std::string value = trim(lines[i].substr(colon + 1));

        if (key == "status"){
            res.statusLine = "HTTP/1.1 " + value;
        }
        else{
            res.headers[key].push_back(value);
        }
    }

    if (res.statusLine.empty())
        res.statusLine = "HTTP/1.1 200 OK";

    res.body = body;

    return res;
}


std::map<std::string, std::vector<std::string> > WebServer::createEnvp(const SocketClient* client, const std::string& path){

    std::map<std::string, std::vector<std::string> > env;
    const HttpRequest& req = client->getRequest();
    
    std::string contentType;
    std::map<std::string, std::vector<std::string> > headers = req.getHeaders();
    std::map<std::string, std::vector<std::string> >::iterator it = headers.find("content-type");

    if (it != headers.end() && !it->second.empty())
        contentType = it->second[0];

    env["REQUEST_METHOD"].push_back(req.getMethod());
    env["QUERY_STRING"].push_back(req.getOriginQuery());
    env["CONTENT_LENGTH"].push_back( longToString(req.getContentLength()) );
    env["CONTENT_TYPE"].push_back(contentType);
    env["SCRIPT_NAME"].push_back(path);
    env["PATH_INFO"].push_back(extractPathInfo(path, req.getOriginPath()));
    env["SERVER_PROTOCOL"].push_back("HTTP/1.1");
    env["GATEWAY_INTERFACE"].push_back("CGI/1.1");

    return env;
}

std::string WebServer::extractPathInfo(const std::string& uri, const std::string& scriptPath) {
    if (uri.size() <= scriptPath.size())
        return "";

    if (uri.compare(0, scriptPath.size(), scriptPath) != 0)
        return "";

    return uri.substr(scriptPath.size());
}

char**          WebServer::convertMapToChar(const std::map<std::string, std::vector<std::string> >& env) {
    char** res = new char*[env.size() + 1];

    for (size_t i = 0 ; i <= env.size(); ++i){
        res[i] = NULL;
    }
    
    size_t i = 0; 
    
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = env.begin(); it != env.end(); ++it) { 
        std::string line = it->first + "="; 
        const std::vector<std::string>& values = it->second; 
        for (size_t j = 0; j < values.size(); j++) { 
            line += values[j]; 
                if (j + 1 < values.size())
                    line += ", ";
            } 
        res[i] = strdup(line.c_str());
        if (res[i] == NULL){
            freeTab(&res);
            return NULL;
        }
        i++;
    }
    res[i] = NULL; 
    return res; 
}
