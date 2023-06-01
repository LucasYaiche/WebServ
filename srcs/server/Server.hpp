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

        void	               		run();
        void                    	close_sockets();
        std::pair<bool, Location>   check_location(ServInfo& current_port, const std::string& request_location);
		bool 						is_method_valid(std::pair<bool, Location> result, const std::string& method);
        int                     	handle_cgi_request(int client_fd, const Request& request, std::vector<ServInfo> ports);


	private:

		std::vector<Socket>     	_server_sockets;
        std::vector<pollfd>     	_fds;
        std::vector<ServInfo>   	_ports;
};

#endif
