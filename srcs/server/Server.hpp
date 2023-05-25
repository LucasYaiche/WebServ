#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <poll.h>
#include <string>
#include <vector>

#include "../socket/Socket.hpp"
#include "../cgi/cgi.hpp"
#include "../methods/methods.hpp"
#include "../parsing/Config.hpp"
#include "../parsing/Location.hpp"
#include "../parsing/ServInfo.hpp"

class Server
{
    public:
        Server(std::vector<ServInfo> ports);
        ~Server();

        void	                run();
        int                     handle_cgi_request(int client_fd, const Request& request);


	private:

		std::vector<Socket>     _server_sockets;
        std::vector<pollfd>     _fds;
        std::vector<ServInfo>   _ports;
};

#endif
