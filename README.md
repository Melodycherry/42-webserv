# 🌐 WEBSERV

*This project has been created as part of the 42 curriculum by `hlichten` and `mlaffita`*

## 📖 Description   

Webserv is a HTTP/1.1 server developed in C++ 98.  
The goal is to recreate NGINX (in a simplified way) : write a program that listens on a port, receives HTTP requests from browsers (Chrome, Safari, curl), parses them and returns a response (an HTML page, a file, or an error)    

🛠️ **Key Features:**  
- Multiplexing ( uses a single multiplexer (`poll`) to monitor all I/O operations (read/write) simultaneously.
- HTTP Protocol Support (fully handles GET, POST, and DELETE methods)
- Custom Configuration ( Nginx-like configuration files)
- CGI Execution ( python, php, ...)
- File management ( file uploads, static website hosting, and directory listing (autoindex))
- Designed to remain operational under stress and handle client disconnections properly 


## 💻 Instructions  

You will find below the details for compilation, execution and a few example of testing commands.  

➜ **Compilation**   

The project includes a standard Makefile. To compile the executable, run:
```
make
```

➜ **Execution**   

You must provide a configuration file as an argument. If no file is provided, the server will exit with an error.
```
./webserv [config_file.conf]
```

➜  **Testing**  

Once the server is running, you can test its functionality via:  
1- Web Browsers: Navigate to http://localhost:8080 (or configured port)  
2- Curl: Use curl -v localhost:8080  

| Test | Commande | output |
|---|---|---|
| GET  | `curl -v http://localhost:8080/` | `200 OK` |
| GET with file | `curl -v http://localhost:8080/index.html` | Affichage du fichier `index.html` |
| GET non-existing | `curl -v http://localhost:8080/nop.html` | `404 Not Found` |
| GET (index / autoindex) | `curl -v http://localhost:8080/Assets/` | Si autoindex = `on` → affichage du dossier, sinon → `403 Forbidden` |
| HEAD (headers uniquement) | `curl -vI http://localhost:8080/index.html` | Retourne uniquement les headers. Le body doit être totalement absent, même si `Content-Length > 0` |
|...|...|...|


## 📚 Resources  

🔵 **Articles:**    
*webserv: Building a Non-Blocking Web Server in C++98* :   
https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7    
*Writing an Nginx-like web server from scratch in C++* :   
https://www.alimnaqvi.com/blog/webserv    

🔵 **Videos:**  
*Web Server Concepts and Examples* :   
https://www.youtube.com/watch?app=desktop&v=9J1nJOivdyw&pp=iggCQAE%3D    
*C++ Web Server from Scratch | Part 1: Creating a Socket Object* :    
https://www.youtube.com/watch?v=YwHErWJIh6Y    

🔵 **Tutorial**  
*Code Quoi | Programmation réseau via socket en C* :   
https://www.codequoi.com/programmation-reseau-via-socket-en-c/    
*Geeks for geeks | Socket Programming in C* :   
https://www.geeksforgeeks.org/c/socket-programming-cc/    

🔵 **AI was used for:**    
- Test Automation: Generating Python and bash tests scripts
- Code documentation and general explanation of some concepts 
