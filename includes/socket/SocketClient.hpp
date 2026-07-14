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

#ifndef SOCKETCLIENT_HPP
#define SOCKETCLIENT_HPP


#define MAX_HEADER_SIZE 8192 //8KB
#define MAX_HEADER_COUNT 100 //-USUAL
#define MAX_HEADER_LINE_SIZE 4096 //4KB

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <ctime>
#include "SocketServer.hpp"


#include "../http/HttpRequest.hpp"

enum ChunkState {
    CHUNK_SIZE,
    CHUNK_DATA,
    CHUNK_CRLF,
    CHUNK_DONE,
    CHUNK_ERROR
};

enum RequestState {
    READING,
    HEADERS_PARSED,
    BODY_READING,
    READY,
    PROCESSING,
    RESPONDING,
    DONE,
    ERROR
};

class SocketClient {

	private:

		int				_fd;
		long			_bytesRead;
		long			_bytesPending;
		std::string		_buffer;
		ChunkState		_chunkState;
		SocketServer*	_server;
		
		HttpRequest _request;
		
		struct sockaddr_storage _addr; 
		
		SocketClient(const SocketClient& other);
		SocketClient& operator=(const SocketClient& other);
		
		public:
		
		bool			ignoreBody;
		bool        	headerParsed;
		bool        	requestCompleted;
		bool        	contentLength;
		bool        	chunked;
		bool			keepAlive;
		int				errorCode;
		time_t			lastActivity;
		RequestState 	state;
		
		// construtor & destructors 
		SocketClient();
		SocketClient(int fd, struct sockaddr_storage addr, SocketServer* serverPtr);
		~SocketClient();
		
		
		// Getters and setters
		int					getFd() const;
		const HttpRequest&	getRequest() const;
		HttpRequest& 		getRequest();
		const std::string&	getBuffer() const;
		long				getBytes();
		const SocketServer*	getServer() const;
		
		//parsing poll request 
		void	addBytes(long bytes);
		void	appendBuffer(const std::string& str);
		void	parseRequest();
		
		// parsing firstLine & Headers 
		void	parseFirstLine(std::string &buffer, size_t &position);
		void	parseHeaders(std::string &buffer, size_t &position);
		
		// Parsing Body 
		void	parsingNoBody();
		void	parsingChunked();
		void	parsingContentLength();

		// checks Parsing 
		bool	isValidURI();
		bool	isValidMethod();
		bool	isValidVersion();
		void	validateHeaders(int headerCount, size_t totalSize);
		bool	isValidHeadersContent();
		size_t	getHeaderCount(const std::map<std::string, std::vector<std::string> >& headers, const std::string& name);
		bool	isDone() const;

		void	defineBodyType();
		void	cleanBuffer();
};

#endif