#include "Server.hpp"


Server::Server(std::vector<ServInfo> ports) : _ports(ports), _buffer_size(0)
{   
    for(size_t i=0; i < ports.size(); i++)
    {
        Socket socket;
        socket.create_socket();
        socket.set_non_blocking();
        if (!socket.bind("127.0.0.1", ports[i].getPort())) 
        {
            throw std::runtime_error("Failed to bind server socket.");
        }

        if (!socket.listen(42)) // A voir si on peut pas le preset dans les fichiers de config
        {
            throw std::runtime_error("Failed to listen on server socket.");
        }

        _server_sockets.push_back(socket);

        // Add the server socket to the fds vector with the POLLIN event
        pollfd server_pollfd;
        server_pollfd.fd = socket.get_fd();
        server_pollfd.events = POLLIN;
        server_pollfd.revents = 0;
        _fds.push_back(server_pollfd);


        //Size of buffer
        if (ports[i].getBody_size() > _buffer_size) {
            _buffer_size = ports[i].getBody_size();
        }
    }
    _buffer = new char[_buffer_size];
    if (!_buffer)
        throw std::runtime_error("failed to allocate buffer.");
}

Server::~Server() 
{
    delete[] _buffer;
}

void Server::close_sockets() 
{
        for (size_t i = 0; i < _server_sockets.size(); ++i) {
            if (_server_sockets[i].get_fd() != -1) {
                _server_sockets[i].close();
            }
        }
        for (size_t i = 0; i < _fds.size(); ++i) {
            Socket client_socket;
            client_socket.set_socket_fd(_fds[i].fd);
            if (client_socket.get_fd() != -1) {
                client_socket.close();
            }
        }
}

bool Server::is_method_valid(std::pair<bool, Location> result, const std::string& method, ServInfo port) 
{
    // Check location level if a location exists
    if(result.first)
    {
        Location port_location = result.second;

        const std::vector<std::string>& location_methods = port_location.getMethods();
        if (std::find(location_methods.begin(), location_methods.end(), method) == location_methods.end())
            return false;
        else
            return true;
    }
    // Check port level first
    const std::vector<std::string>& port_methods = port.getMethods();
    if (std::find(port_methods.begin(), port_methods.end(), method) == port_methods.end())
    {
        return false;
    }    

    return true;
}

void    Server::delete_socket(Socket client_socket, size_t &i)
{
    client_socket.close();
    _fds.erase(_fds.begin() + i);
    i--;
}


void Server::run() 
{
    while (true) 
    {

        int poll_result = poll(&_fds[0], _fds.size(), -1);

        if (poll_result <= 0) 
        {
            std::cerr << "poll() failed: " << std::endl;
            if (!poll_result)
            {
                std::cerr << "poll time out" << std::endl;
            } 
            continue;
        }

        for (size_t i = 0; i < _fds.size(); ++i) 
        {
            if (_fds[i].revents & POLLIN) 
            {
                bool is_server_fd = false;
                for(size_t j = 0; j < _server_sockets.size(); j++)
                {
                    if (_fds[i].fd == _server_sockets[j].get_fd()) 
                    {
                        is_server_fd = true;

                        sockaddr_in client_address;
                        socklen_t client_addr_len = sizeof(client_address);
                        int client_socket_fd = _server_sockets[j].accept((sockaddr*)&client_address, &client_addr_len);
                        if (client_socket_fd == -1)
                        {
                            std::cerr << "accept() failed: " << std::endl;
                            continue;
                        }
                        else
                        {
                            Socket client_socket;
                            client_socket.set_socket_fd(client_socket_fd);
                            if (client_socket.set_non_blocking() == -1)
                                continue;

                            int n = 1;
                            if (setsockopt(client_socket.get_fd(), SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)) == -1)
                            {
                                std::cerr << "setsockopt socket error" << std::endl;
                                continue;
                            }

                            pollfd client_pollfd;
                            client_pollfd.fd = client_socket_fd;
                            client_pollfd.events = POLLIN;
                            client_pollfd.revents = 0;
                            _fds.push_back(client_pollfd);
                        }
                        break;
                    }
                }

                if (!is_server_fd)
                {
                    Socket client_socket;
                    client_socket.set_socket_fd(_fds[i].fd);
                    ssize_t bytes_received = 0;
                    int port_nb = client_socket.get_local_port();
                    ServInfo current_port;
                    for(size_t i=0; i < _ports.size(); i++)
                    {
                        if(_ports[i].getPort() == port_nb)
                        {
                            current_port = _ports[i];
                            break;
                        }
                    }

                    if (!_buffer)
                    {
                        send_error_response(client_socket.get_fd(), 500, "Internal Server Error", current_port);
                        delete_socket(client_socket, i);
                        continue;
                    }

                    bytes_received = client_socket.recv(_buffer, _buffer_size);

                    if (bytes_received < 0) {
                        // recv() would block, treat as temporary error and continue to next iteration
                        // no need to delete socket in this case because of the non blocking mode
                        continue;
                    }
                    else if (bytes_received == 0) {
                        // The client has closed the connection
                        delete_socket(client_socket, i);
                        continue;
                    }

                    Request request;
                    if (request.parse(_buffer, bytes_received) == -1)
                    {
                        send_error_response(client_socket.get_fd(), 400, "Bad Request", current_port);
                        delete_socket(client_socket, i);
                        continue;
                    }
                        
                    std::pair<bool, Location> result = check_location(current_port, request.get_uri());

                    std::string method = request.get_method();
                    bool methodValid = is_method_valid(result, method, current_port);

                    Socket method_socket;
                    method_socket.set_socket_fd(_fds[i].fd);
                    method_socket.set_non_blocking();

                    if (request.is_cgi() && methodValid)
                    {
                        if (handle_cgi_request(method_socket, request, _ports) == -1)
                        {
                            send_error_response(method_socket.get_fd(), 500, "Internal Server Error", current_port);
                            delete_socket(client_socket, i);
                            continue;
                        }
                    }
                    else if(methodValid)
                    {    
                        if (method == "GET") 
                        {
                            if (handle_get_request(method_socket, request, current_port) == -1)
                            {
                                delete_socket(client_socket, i);
                                continue;
                            }
                        }
                        else if (method == "POST") 
                        {
                            if (handle_post_request(method_socket, request, current_port) == -1)
                            {
                                delete_socket(client_socket, i);
                                continue;
                            }
                        }
                        else if (method == "DELETE") 
                        {
                            if (handle_delete_request(method_socket, request, current_port) == -1)
                            {
                                delete_socket(client_socket, i);
                                continue;
                            }
                        }
                    }
                    else 
                    {
                        send_error_response(method_socket.get_fd(), 405, "Method not allowed", current_port);
                        delete_socket(client_socket, i);
                        continue;
                    }
                }
            }
            usleep(4000);
        }
    }
    close_sockets();
}

int Server::handle_cgi_request(Socket method_socket, const Request& request, std::vector<ServInfo> ports)
{
    // Extract the path and the query string from the URI
    std::string uri = request.get_uri();
    std::size_t delimiter_pos = uri.find('?');
    std::string path = uri.substr(0, delimiter_pos);
    std::string query_string = delimiter_pos == std::string::npos ? "" : uri.substr(delimiter_pos + 1);
    
    // Create a new CGI instance and run the script
    CGI cgi(path, query_string, request, ports, method_socket.get_fd());
    std::string script_output;
    int script_status = cgi.run_cgi_script(script_output);

    if(script_status != 0)
    {
        
        std::cerr << "Error: could not run CGI script\n";
        return -1;
    }

    // Send the script output back to the client
    return (method_socket.send(script_output.c_str(), script_output.size()));
}
