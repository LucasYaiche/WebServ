#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <poll.h>
#include <string>
#include <vector>

#include "../socket/Socket.hpp"
#include "../cgi/cgi.hpp"
#include "../methods/methods.hpp"

class Server
{
    public:
        Server(const std::string& address, int port);
        ~Server();

        void	                run();
        void                    handle_cgi_request(int client_fd, const Request& request);


	private:

		Socket                  _server_socket;
        std::vector<pollfd>     _fds;	
};

#endif
