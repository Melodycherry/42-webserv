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

#include "SocketClient.hpp"
#include "WebServer.hpp"
#include "../utils/utilsGeneral.hpp"
#include <sstream>
#include "Exceptions.hpp"

/* ************************************************** */
/* construtor & destructors                           */
/* ************************************************** */

SocketClient::SocketClient() : _fd(-1), _bytesRead(0), _bytesPending(0), _buffer(""), _chunkState(CHUNK_SIZE), _server(NULL), ignoreBody(false), headerParsed(false), requestCompleted(false), contentLength(false), chunked(false), keepAlive(true), errorCode(0), lastActivity(std::time(NULL)), state(READING){
}
SocketClient::SocketClient(int fd, struct sockaddr_storage addr, SocketServer* serverPtr) : _fd(fd), _bytesRead(0), _bytesPending(0), _buffer(""), _chunkState(CHUNK_SIZE), _server(serverPtr), _addr(addr), ignoreBody(false), headerParsed(false), requestCompleted(false), contentLength(false), chunked(false), keepAlive(true), errorCode(0), lastActivity(std::time(NULL)), state(READING){}
SocketClient::~SocketClient(){}

/* ************************************************** */
/* getters & setters.                                 */
/* ************************************************** */

int					SocketClient::getFd() const { return (_fd); }
long				SocketClient::getBytes() { return (_bytesRead); }
const HttpRequest&	SocketClient::getRequest() const { return (_request); }
HttpRequest& 		SocketClient::getRequest() { return _request; }
const std::string&	SocketClient::getBuffer() const{ return (_buffer); }
const SocketServer*	SocketClient::getServer() const { return (_server); }

/* ************************************************** */
/* parsing de la request de Poll                      */
/* ************************************************** */

void	SocketClient::addBytes(long bytes){ _bytesRead += bytes; }
void	SocketClient::appendBuffer(const std::string& str){ _buffer += str; }

void	SocketClient::parseRequest(){
	size_t headerEnd = _buffer.find("\r\n\r\n");
	size_t position = 0;
	
	if (headerEnd == std::string::npos)
		return ;

	parseFirstLine(_buffer, position);
	parseHeaders(_buffer, position);


	_buffer.erase(0, headerEnd + 4);

	if (_request.getMethod() == "POST")
		defineBodyType();

	headerParsed = true;
	state = (chunked || contentLength) ? BODY_READING : READY;
}

void	SocketClient::defineBodyType(){

	const std::map<std::string, std::vector<std::string> >& headers = _request.getHeaders();
	const std::vector<const ServerConfig*>& configs = getServer()->getServers();
	const ServerConfig* theConfig = configs[0]; 
    std::string host;
    
	std::map<std::string, std::vector<std::string> >::const_iterator it = headers.find("host");
    if (it != headers.end())
        host = it->second[0];
    std::string cleanHost = host.substr(0, host.find(':'));
    for (size_t i = 0; i < configs.size(); i++) {
        if (configs[i]->serverName == cleanHost)
            theConfig = configs[i];
    }

	std::map<std::string, std::vector<std::string> >::const_iterator itCL;
	itCL = headers.find("content-length");
	if (itCL != headers.end()){
		contentLength = true;

		_request.setContentLength(stringToLong(itCL->second[0].c_str()));
		if (_request.getContentLength() < 0){
			throw ResponseException(_fd, 405);
		}
		
		if (_request.getContentLength() < 0 || static_cast<size_t>(_request.getContentLength()) > theConfig->maxBodySize)
			throw ResponseException(_fd, 413);
	}
	else {
		contentLength = false;
		_request.setContentLength(0);
	}

	chunked = false;
	std::map<std::string, std::vector<std::string> >::const_iterator itTE;
	itTE = headers.find("transfer-encoding");
	if (itTE != headers.end()){
		std::string value = toLower(itTE->second[0]);
		if (value.find("chunked") != std::string::npos)
			chunked = true;
	}

	if (!contentLength && !chunked){
		throw ResponseException(_fd, 405);
	}

	if (chunked)
		contentLength = false;

	if (chunked || contentLength)
		_request.openBodyFile(_fd);
}

/* ************************************************** */
/* parsing firstLine & Headers                        */
/* ************************************************** */

void SocketClient::parseFirstLine(std::string &buffer, size_t &pos)
{
	size_t end = buffer.find("\r\n", pos);
	if (end == std::string::npos){
		throw ResponseException(_fd, 400);
	}

	std::string line = buffer.substr(pos, end - pos);

	size_t s1 = line.find(' ');
	if (s1 == std::string::npos){
		throw ResponseException(_fd, 400);
	}

	size_t s2 = line.find(' ', s1 + 1);
	if (s2 == std::string::npos){
		throw ResponseException(_fd, 400);
	}
	std::string method = line.substr(0, s1);

	std::string uri = line.substr(s1 + 1, s2 - s1 - 1);

	if (uri.size() > MAX_URI_SIZE){
		throw ResponseException(_fd, 414);
	}

	std::string version = line.substr(s2 + 1);
	
	_request.setMethod(method);
	_request.setUri(uri);
	_request.setVersion(version);

	if (!isValidURI())
		throw ResponseException(_fd, 400);

	if (!isValidVersion())
		throw ResponseException(_fd, 400);

	pos = end + 2;
}

void SocketClient::parseHeaders(std::string &buffer, size_t &pos)
{
	size_t end = buffer.find("\r\n\r\n", pos);
	if (end == std::string::npos)
		throw ResponseException(_fd, 400);

	int count = 0;
	size_t total = 0;

	while (pos < end)
	{
		size_t lineEnd = buffer.find("\r\n", pos);
		if (lineEnd == std::string::npos || lineEnd > end)
			break;

		std::string line = buffer.substr(pos, lineEnd - pos);

		if (line.empty()) {
			pos = lineEnd + 2;
			continue;
		}

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			throw ResponseException(_fd, 400);

		std::string key = line.substr(0, colon);
		std::string value = trim(line.substr(colon + 1));

		if (key.empty())
			throw ResponseException(_fd, 400);

		size_t space = key.find(' ');
		if (space != std::string::npos)
			throw ResponseException(_fd, 400);

		_request.setHeaders(toLower(key), value);

		count++;
		total += line.size();

		pos = lineEnd + 2 ;
	}

	validateHeaders(count, total);

	state = HEADERS_PARSED;
}


/* ************************************************** */
/* Parsing Body                                       */
/* ************************************************** */

void	SocketClient::parsingNoBody(){
	requestCompleted = true;
}

bool isDigits(const std::string& str)
{
	if (str.empty())
		return false;

	for (size_t i = 0; i < str.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(str[i])))
			return false;
	}

	return true;
}

void SocketClient::parsingChunked()
{
	while (true) {
		if (_chunkState == CHUNK_SIZE) {
			size_t pos = _buffer.find("\r\n");

			if (pos == std::string::npos) {
				return;
			}

			std::string line = _buffer.substr(0, pos);
			for (size_t i = 0; i < line.size(); i++) {
				if (!isHex(line[i])) {
					throw ResponseException(_fd, 400);
				}
			}
			
			_bytesPending = hexToLong(line);

			if (_bytesPending < 0)
				throw ResponseException(_fd, 400);

			_buffer.erase(0, pos + 2);

			if (_bytesPending == 0) {
				_chunkState = CHUNK_DONE;
				continue;
			}
			_chunkState = CHUNK_DATA;
		}

		else if (_chunkState == CHUNK_DATA)
		{
			if ((long)_buffer.size() < _bytesPending) {
				throw ResponseException(_fd, 400);
			}
			std::string chunk = _buffer.substr(0, _bytesPending);

			if (_bytesRead + _bytesPending > DEFAULT_MAX_BODY_SIZE) { 
				throw ResponseException(_fd, 413);
			}
			// METTRE L AUTRE
			_request.writeBody(chunk, _fd);
			_bytesRead += _bytesPending;
			_buffer.erase(0, _bytesPending);
			_chunkState = CHUNK_CRLF;
		}

		else if (_chunkState == CHUNK_CRLF)
		{
			if (_buffer.size() < 2) {
				return;
			}

			if (_buffer.compare(0, 2, "\r\n") != 0) {
				throw ResponseException(_fd, 400);
			}
			_buffer.erase(0, 2);
			_chunkState = CHUNK_SIZE;
		}

		else if (_chunkState == CHUNK_DONE)
		{
			if (_buffer.size() < 2) 
				return;

			if (_buffer.compare(0, 2, "\r\n") != 0) {
				throw ResponseException(_fd, 400);
			}

			_buffer.erase(0, 2);

			requestCompleted = true;
			return;
		}

		else {
			throw ResponseException(_fd, 400);
		}
	}
}

void SocketClient::parsingContentLength() {
	size_t size = _buffer.size();
	size_t remaining = _request.getContentLength() - _bytesRead;

	if (size > remaining)
		size = remaining;

	if (size > 0) {
		std::string chunk = _buffer.substr(0, size);

		_request.writeBody(chunk, _fd);
		_bytesRead += size;
		_buffer.erase(0, size);
	}

	if (_bytesRead == _request.getContentLength()) {
		requestCompleted = true;
		_request.closeBodyFile();
	}
}

/* ************************************************** */
/* checks Parsing                                     */
/* ************************************************** */

void SocketClient::validateHeaders(int headerCount, size_t totalSize) {

	const std::map<std::string, std::vector<std::string> >& header = _request.getHeaders();

	if (headerCount > MAX_HEADER_COUNT)
		throw ResponseException(_fd, 431);

	if (totalSize > MAX_HEADER_SIZE)
		throw ResponseException(_fd, 431);

	if (header.find("host") == header.end()){
		throw ResponseException(_fd, 400);
	}

	if (!isValidHeadersContent()){
		throw ResponseException(_fd, 400);
	}

	std::map<std::string, std::vector<std::string> >::const_iterator it = header.find("content-length");
	if (it != header.end()) {
		if (it->second[0].find_first_not_of("0123456789") != std::string::npos){
			throw ResponseException(_fd, 400);
		}
	}
}

bool	SocketClient::isValidHeadersContent() {
	const std::map<std::string, std::vector<std::string> >& header = _request.getHeaders();

	if (getHeaderCount(header, "host") > 1)
		return false;
	
	if (getHeaderCount(header, "content-length") > 1)
		return false;

	if (getHeaderCount(header, "transfert-encoding") > 1)
		return false;

	return true;
}

size_t	SocketClient::getHeaderCount(const std::map<std::string, std::vector<std::string> >& headers, const std::string& name) {
	std::map<std::string, std::vector<std::string> >::const_iterator it = headers.find(name);
	if (it == headers.end())
		return 0;

	return it->second.size();
}

bool	SocketClient::isValidURI() {
	const std::string& uri = _request.getUri();

	if (uri.empty())
		return false;

	if (uri[0] != '/')
		return false;

	for (size_t i = 0; i < uri.size(); i++) {
		unsigned char c = uri[i];
		if (c < 32 || c == 127)
			return false;
	}

	return true;
}

bool	SocketClient::isValidMethod(){
	if (_request.getMethod() == "POST" || _request.getMethod() == "GET" || _request.getMethod() == "DELETE")
		return true;
	return false;
}

bool	SocketClient::isValidVersion(){
	if (_request.getVersion() != "HTTP/1.1")
		return false;
	return true;
}

bool	SocketClient::isDone() const {
	return (_chunkState == CHUNK_DONE);
}

void	SocketClient::cleanBuffer(){
	size_t pos = _buffer.find("\r\n\r\n");

	_buffer.erase(0, pos + 4);
}

