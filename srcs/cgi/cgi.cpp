#include "cgi.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

CGI::CGI(const std::string& script_path, const std::string& query_string, const Request& request, std::vector<ServInfo> ports, int client_fd) 
		: _script_path(script_path), _query_string(query_string), _request(request)
{
	// Get the port the client is connected to
    Socket client_socket;
    client_socket.set_socket_fd(client_fd);
    int port = client_socket.get_local_port();

	//Get the correct data set of the port
	for(size_t i=0; i < ports.size(); i++)
	{
		if(ports[i].getPort() == port)
		{
			_port_info = ports[i];
			break;
		}
	}
}

CGI::~CGI() {}

std::string CGI::run_cgi_script()
{
	std::string base_path = _port_info.getRoot();
	std::string full_script_path = base_path + _script_path;

	// Ensure the script file exists and is executable
	if (access(full_script_path.c_str(), X_OK) == -1) 
	{
		throw std::runtime_error("CGI script not found or not executable: " + full_script_path);
	}
	// Set up the environment for the CGI script
	if (setenv("QUERY_STRING", _query_string.c_str(), 1) == -1) 
	{
		throw std::runtime_error("Failed to set QUERY_STRING environment variable");
	}
	if (setenv("REQUEST_METHOD", _request.get_method().c_str(), 1) == -1) 
	{
		throw std::runtime_error("Failed to set REQUEST_METHOD environment variable");
	}
	if (setenv("CONTENT_TYPE", _request.get_headers().at("Content-Type").c_str(), 1) == -1) 
	{
		throw std::runtime_error("Failed to set CONTENT_TYPE environment variable");
	}
	if (setenv("CONTENT_LENGTH", std::to_string(_request.get_body().size()).c_str(), 1) == -1) 
	{
		throw std::runtime_error("Failed to set CONTENT_LENGTH environment variable");
	}
	if (setenv("SERVER_PORT", std::to_string(_port_info.getPort()).c_str(), 1) == -1)
	{
		throw std::runtime_error("Failed to set SERVER_PORT environment variable");
	}

	// Execute the CGI script and capture its output
	std::string command = "/usr/bin/perl " + full_script_path;
	FILE* pipe = popen(command.c_str(), "r+");
	if (!pipe) 
	{
		throw std::runtime_error("Failed to execute CGI script: " + full_script_path);
	}

	// If this is a POST request, write the request body to the CGI script's standard input
	if (_request.get_method() == "POST") 
	{
		size_t written = fwrite(_request.get_body().data(), sizeof(char), _request.get_body().size(), pipe);
		if (written < _request.get_body().size()) 
		{
			std::cerr << "Error: writing to file\n";
			exit(1);
    	}
		if (fflush(pipe) == EOF) { // Ensure that the input data is sent to the script
            std::cerr << "Error: could not fflush\n";
            exit(1);
        }
    }

	// Read the output from the process
	char buffer[5242880];
	std::string result = "";
	while (fgets(buffer, 5242880, pipe) != NULL) 
	{
		result += buffer;
	}

	// Close the process and check the status
	int status = pclose(pipe);
	if (status == -1) 
	{
		throw std::runtime_error("Failed to close CGI script process");
	}
	else if (WEXITSTATUS(status) != 0) 
	{
		throw std::runtime_error("CGI script exited with non-zero status: " + full_script_path);
	}

	// Clear the environment variables
	unsetenv("QUERY_STRING");
	unsetenv("REQUEST_METHOD");
	unsetenv("CONTENT_TYPE");
	unsetenv("CONTENT_LENGTH");
	unsetenv("SERVER_PORT");

	// Parse the headers and content from the CGI script's output
	std::string headers;
	std::string content;
	size_t separator_pos = result.find("\r\n\r\n");
	if (separator_pos != std::string::npos) 
	{
		headers = result.substr(0, separator_pos);
		content = result.substr(separator_pos + 4);  // skip over "\r\n\r\n"
	} 
	else 
	{
		// If there's no "\r\n\r\n", assume the entire output is content
		content = result;
	}

	// Extract the Content-Type from the headers (if present)
	std::string content_type = "text/html; charset=UTF-8";  // default value
	size_t content_type_pos = headers.find("Content-Type: ");
	if (content_type_pos != std::string::npos) 
	{
		size_t content_type_end_pos = headers.find("\r\n", content_type_pos);
		if (content_type_end_pos != std::string::npos) 
		{
			content_type = headers.substr(content_type_pos + 14, content_type_end_pos - (content_type_pos + 14));  // skip over "Content-Type: "
		}
	}

	// Extract the Status from the headers (if present)
	std::string status_code = "200 OK";  // Set default value
	size_t status_pos = headers.find("Status: ");
	if (status_pos != std::string::npos) 
	{
		size_t status_end_pos = headers.find("\r\n", status_pos);
		if (status_end_pos != std::string::npos) 
		{
			status_code = headers.substr(status_pos + 8, status_end_pos - (status_pos + 8));  // skip over "Status: "
		}
	}

	// Construct the HTTP response
	std::string response = "HTTP/1.1 " + status_code + "\r\n";
	response += headers + "\r\n";
	response += "Content-Type: " + content_type + "\r\n";  // use the parsed content type
	response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
	response += "\r\n";
	response += content;

	// std::cout << response << std::endl;

	return response;
}
