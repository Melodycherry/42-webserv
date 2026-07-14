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
#include "Exceptions.hpp"
#include "LocationConfig.hpp"
#include "ParserConfig.hpp"
#include "ServerConfig.hpp"
#include <iostream>
#include <csignal>

static WebServer* g_server = 0;

void handle_sigint(int signum) {
    if (signum)
        g_server->_running = false;
}

int main (int ac, char **av)
{
    if (ac != 2) {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }

    try {
        ParserConfig parser;
        parser.parse(av[1]);

        WebServer webserver(parser.getServers());

        g_server = &webserver;
        signal(SIGINT, handle_sigint);

        webserver.pollLoop();

        g_server = 0;
    }
    catch (const ParseException &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception &e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

