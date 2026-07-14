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

#include <iostream>
#include <fstream>
#include <map>

class HttpRequest
{
    private :
    // REQUEST LINE
    std::string _method;
    std::string _uri;
    std::string _version;

    std::string _originPath;
    std::string _originQuery;

    std::string _path;
    std::map<std::string, std::vector<std::string> > _query;

    //HEADERS
    long    _contentLength;
    std::map<std::string, std::vector<std::string> > _headers;
    
    //BODY 
    std::string _bodyFilePath;
    std::ofstream _bodyFile;
    
    HttpRequest(const HttpRequest &other);
    HttpRequest& operator=(const HttpRequest &other);
    
    public :

    HttpRequest();
    ~HttpRequest();

    const   std::string& getMethod() const;
    const   std::string& getUri() const;
    const   std::string& getVersion() const;
    const   std::string& getBodyPath() const;
    const   std::string& getPath() const;
    const   std::string& getOriginPath() const;
    const   std::string& getOriginQuery() const;
    const   std::map<std::string, std::vector < std::string> >& getHeaders() const;
    long    getContentLength() const;

    void    setMethod(std::string);
    void    setUri(std::string);
    void    setVersion(std::string);
    void    setBodyPath(std::string);
    void    setPath(std::string);
    void    setContentLength(long length);
    void    setHeaders(const std::string& key, const std::string& value);
    void    setQuery(std::map<std::string, std::vector <std::string> > query);

    void    setOriginPath(std::string);
    void    setOriginQuery(std::string);

    void    writeBody(const std::string& str, int fd);
    void    closeBodyFile();
    void    openBodyFile(int fd);
};