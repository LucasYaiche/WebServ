#ifndef METHODS_HPP
#define METHODS_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../request/request.hpp"


int handle_get_request(int client_socket, const Request& request);
int handle_post_request(int client_socket, const Request& request);
int handle_delete_request(int client_socket, const Request& request);
int send_error_response(int client_socket, int status_code, const std::string& status_message);

#endif
