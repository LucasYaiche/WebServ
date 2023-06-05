#ifndef METHODS_HPP
#define METHODS_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "../request/request.hpp"
#include "../parsing/ServInfo.hpp"


int                         handle_get_request(int client_socket, const Request& request, ServInfo port);
int                         handle_post_request(int client_socket, const Request& request, ServInfo port);
int                         handle_delete_request(int client_socket, const Request& request, ServInfo port);
int                         send_error_response(int client_socket, int status_code, const std::string& status_message);
int                         handle_directory_request(int client_socket, const std::string& directory_path);
std::pair<bool, Location>   check_location(ServInfo& current_port, const std::string& request_location);



#endif
