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
		bool 						is_method_valid(std::pair<bool, Location> result, const std::string& method, ServInfo port);
        int                     	handle_cgi_request(Socket method_socket, const Request& request, std::vector<ServInfo> ports);
		void    					delete_socket(Socket client_socket, size_t &i);


	private:

		std::vector<Socket>     	_server_sockets;
        std::vector<pollfd>     	_fds;
        std::vector<ServInfo>   	_ports;
		char*						_buffer;
		size_t 						_buffer_size;
};

#endif
