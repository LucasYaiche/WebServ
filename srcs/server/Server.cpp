#include "Server.hpp"


Server::Server(const std::string& address, int port)
{   
    _server_socket.create_socket(AF_INET, SOCK_STREAM, 0, -1);
    _server_socket.set_non_blocking();
    if (!_server_socket.bind(address, port)) 
    {
        throw std::runtime_error("Failed to bind server socket.");
    }

    if (!_server_socket.listen(42)) // A voir si on peut pas le preset dans les fichiers de config
    {
        std::cout << "listen error" << std::endl;
        throw std::runtime_error("Failed to listen on server socket.");
    }
    // Add the server socket to the fds vector with the POLLIN event
    pollfd server_pollfd;
    server_pollfd.fd = _server_socket.get_fd();
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
    _fds.push_back(server_pollfd);

    _fds[0].fd = _server_socket.get_fd();
    _fds[0].events = POLLIN;
}

Server::~Server() {}

void Server::run() 
{
    while (true) 
    {
        int poll_result = poll(&_fds[0], _fds.size(), -1);

        if (poll_result <= 0) 
        {
            if (!poll_result)
            {
                std::cerr << "poll time out" << std::endl;
                continue;

            } 
            else 
            {
                // Handle other poll() errors.
                std::cerr << "poll() failed: " << std::endl;
                break;
            }
        }

        for (size_t i = 0; i < _fds.size(); ++i) 
        {
            if (_fds[i].revents & POLLIN) 
            {
                if (_fds[i].fd == _server_socket.get_fd()) 
                {
                    sockaddr_in client_address;
                    socklen_t client_addr_len = sizeof(client_address);
                    // Accept incoming connections (Step 5)
                    int client_socket_fd = _server_socket.accept((sockaddr*)&client_address, &client_addr_len);
                    if (client_socket_fd == -1)
                    {
                        std::cerr << "accept() failed: " << std::endl;
                        continue;
                    }
                    else {
                        // Set the new client socket to non-blocking mode
                        Socket client_socket;
                        client_socket.set_socket_fd(_fds[i].fd);
                        client_socket.set_non_blocking();
                        int n = 1;
                        if (setsockopt(client_socket.get_fd(), SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)))
                        {
                            std::cout << "setsockopt socket error" << std::endl;
                        }

                        // Add the client socket to the _fds vector with the POLLIN event
                        pollfd client_pollfd;
                        client_pollfd.fd = client_socket_fd;
                        client_pollfd.events = POLLIN; // specifies thta we are interested in the read event (incoming connections)
                        client_pollfd.revents = 0; // reset the returned event field
                        _fds.push_back(client_pollfd);
                    }
                }

                else 
                {
                    Socket client_socket;
                    client_socket.set_socket_fd(_fds[i].fd);
                    ssize_t bytes_received = 0;
                    const size_t buffer_size = 5242880; // 5 MB
                    char* buffer = new char[buffer_size];

                    // Receive data
                    bytes_received = client_socket.recv(buffer, buffer_size - 1);

                    if (bytes_received <= 0) 
                    {
                        // The client closed the connection or there was an error
                        client_socket.close();
                        _fds.erase(_fds.begin() + i);
                        i--; // Decrement the index to account for the removed element
                        continue;
                    }

                        else 
                        {
                            // Process the data (in this case, just echo it back to the client)
                            Request request;
                            request.parse(buffer, bytes_received);

                            // Process the parsed request
                            if (request.is_cgi()) 
                            {
                                std::cout << request.get_headers().at("Content-Type") << std::endl;
                                std::cout << request.get_uri() << std::endl;
                                handle_cgi_request(_fds[i].fd, request);
                            }
                            else if (request.get_method() == "GET") 
                            {
                                handle_get_request(_fds[i].fd, request);
                            }
                            else if (request.get_method() == "POST") 
                            {
                                handle_post_request(_fds[i].fd, request);
                            }
                            else if (request.get_method() == "DELETE") 
                            {
                                handle_delete_request(_fds[i].fd, request);
                            } 
                            else 
                            {
                                // Invalid or unsupported method
                                std::cout << "error method" << std::endl;
                                send_error_response(_fds[i].fd, 405, "Method Not Allowed");
                            }
                            
                            // Send the data back to the client
                            client_socket.send(buffer, bytes_received);
                        }
                }
            }
            usleep(5000);
        }
    }
    _server_socket.close();
}

void Server::handle_cgi_request(int client_fd, const Request& request) 
{
    // Extract the path and the query string from the URI
    std::string uri = request.get_uri();
    std::size_t delimiter_pos = uri.find('?');
    std::string path = uri.substr(0, delimiter_pos);
    std::string query_string = delimiter_pos == std::string::npos ? "" : uri.substr(delimiter_pos + 1);

    // Create a new CGI instance and run the script
    CGI cgi(path, query_string, request);
    std::string script_output = cgi.run_cgi_script();

    // Send the script output back to the client
    Socket client_socket;
    client_socket.set_socket_fd(client_fd);
    client_socket.set_non_blocking();
    client_socket.send(script_output.c_str(), script_output.size());
}
