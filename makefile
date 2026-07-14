NAME = Webserv
CC = c++
INCLUDES = -I includes/ -I includes/config -I includes/socket -I includes/utils
CFLAGS = -Wall -Wextra -Werror -std=c++98 $(INCLUDES)
SRCS = src/main.cpp \
       src/config/ParserConfig.cpp \
       src/config/ServerConfig.cpp \
	   src/config/LocationConfig.cpp \
	   src/socket/SocketClient.cpp \
	   src/socket/SocketServer.cpp \
	   src/http/HttpRequest.cpp \
	   src/http/HttpResponse.cpp \
	   src/webServer/WebServer.cpp \
	   src/webServer/wsUtils/wsResponse.cpp \
	   src/webServer/wsUtils/wsUtilsGet.cpp \
	   src/webServer/wsUtils/wsUtilsPost.cpp \
	   src/webServer/wsUtils/wsUtilsInit.cpp \
	   src/webServer/wsUtils/wsUtilsPath.cpp \
	   src/webServer/wsUtils/wsUtilsPollLoop.cpp \
	   src/webServer/wsUtils/wsUtilsResponse.cpp \
	   src/webServer/wsUtils/wsErrorResponse.cpp \
	   src/utils/utilsGeneral.cpp
OBJDIR = obj
OBJS = $(SRCS:src/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re