#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "../cgi/cgi.hpp"


class Socket
{
    public:

        Socket();
        ~Socket();

        void        create_socket();
        bool        bind(const std::string& address, int port);
        bool        listen(int backlog);
        int         accept(sockaddr *client_address, socklen_t *client_addr_len);
        bool        connect(const std::string& address, int port);
        ssize_t     send(const void* buffer, size_t length);
        ssize_t     recv(void* buffer, size_t length);
        void        close();
        int         set_non_blocking();
        int         get_fd() const{return this->_socket_fd;};
        void        set_socket_fd(int sockfd){  _socket_fd = sockfd;};
        int         get_local_port();
        
    private:    
    
        int         _socket_fd;
        


};

#endif
