#include "Socket.hpp"

Socket::Socket() : _socket_fd(-1) {}

Socket::~Socket() {}

void Socket::create_socket()
{
    if ((_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cerr << "could not create socket\n";
        throw std::runtime_error("Failed to create the socket");
    }

    int n = 1;
    if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)))
    {
        std::cerr << "setsockopt socket error" << std::endl;
        throw std::runtime_error("Failed to use setsockopt with the socket");
    }
}

bool    Socket::bind(const std::string& address, int port)
{
    (void)address;
    sockaddr_in    server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // memset to 0 to make sure there is no garbage data
    server_addr.sin_family = AF_INET; // Specify the type of addresses the socket can communicate with (IPV4 in this case)
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Use of INADDR_ANY to bind any available network interface
    server_addr.sin_port = htons(port);

    if (::bind(_socket_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Error: could not assign a name to socket\n";
        return 0;
    }
    return 1;
}

void Socket::close() 
{
    ::close(_socket_fd);
}

bool    Socket::listen(int backlog)
{
    if (::listen(_socket_fd, backlog) == -1)
    {
        std::cerr << "listen error\n";
        return 0;
    }
    return 1;
}

int Socket::accept(sockaddr *client_address, socklen_t *client_addr_len) 
{
    return ::accept(_socket_fd, client_address, client_addr_len);
}

ssize_t Socket::recv(void* buffer, size_t length) 
{
    return ::recv(_socket_fd, buffer, length, 0);
}

ssize_t Socket::send(const void* buffer, size_t length) 
{
    ssize_t returned = ::send(_socket_fd, buffer, length, 0);
    if(!returned)
    {
        std::cerr << "Client closed the connection.";
        return returned;
    }
    else if (returned == -1)
    {
        std::cerr << "An error occured while sending the data.";
        return returned;
    }
    return returned;

}

int Socket::set_non_blocking()
{
    int flags = fcntl(_socket_fd, F_SETFL, O_NONBLOCK);
    if (flags == -1)
    {
        std::cerr << "non-blocking error" << std::endl;
        return -1;
    }
    return 0;
}


int Socket::get_local_port()
{
    sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if (getsockname(_socket_fd, (struct sockaddr *)&sin, &len) == -1) 
        {
            perror("getsockname");
            return -1;
        }
        else 
        {
            return ntohs(sin.sin_port);
        }
}
