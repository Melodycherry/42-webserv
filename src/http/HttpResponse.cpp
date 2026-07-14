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

#include "../http/HttpResponse.hpp"

HttpResponse::HttpResponse(){}
HttpResponse::HttpResponse(const HttpResponse& other) : statusLine(other.statusLine), headers(other.headers), body(other.body){}
HttpResponse& HttpResponse::operator=(const HttpResponse& other){
    if (this != &other){
        statusLine = other.statusLine;
        headers = other.headers;
        body = other.body;
    }
    return *this;
}
HttpResponse::~HttpResponse(){}

std::string HttpResponse::ResponseToString() const {
    std::stringstream ss;

    ss << statusLine << "\r\n";

    for (std::map<std::string, std::vector<std::string> >::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        const std::string& key = it->first;
        const std::vector<std::string>& values = it->second;

        if (values.empty())
            continue;
        for (size_t i = 0; i < values.size(); ++i){
            ss << key << ": " << values[i] << "\r\n";
        }
    }
    ss << "\r\n";
    ss << body;

    return ss.str();
}

