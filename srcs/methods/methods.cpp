#include "methods.hpp"

int handle_get_request(int client_socket, const Request& request)
{
    const std::string root_directory = "root"; // le root sera different DONC A CHANGER

    // Create the file path
    std::string file_path = root_directory + request.get_uri();

    // Default to "index.html" if the requested URI is a directory
    if (file_path.back() == '/') 
    {
        file_path += "index.html";
    }

    // Read the file content
    std::ifstream file(file_path, std::ios::binary);
    std::string file_content;

    if (file) 
    {
        // Read the file content into a string
        std::ostringstream file_stream;
        file_stream << file.rdbuf();
        file_content = file_stream.str();

        // Send the HTTP response header
        std::string response_header = "HTTP/1.1 200 OK\r\n";
        response_header += "Content-Length: " + std::to_string(file_content.size()) + "\r\n";
        response_header += "\r\n";

        if (send(client_socket, response_header.c_str(), response_header.size(), 0) == -1) {
            std::cerr << "Error: could not send data\n";
            return -1;
        }

        // Send the file content
        if (send(client_socket, file_content.data(), file_content.size(), 0) == -1) {
            std::cerr << "Error: could not send data\n";
            return -1;
        }
    } 
    else // If file not found
    {
        return send_error_response(client_socket, 404, "Not Found");
    }
    return 0;
}

int handle_post_request(int client_socket, const Request& request)
{
    // Set the root directory for serving files
    const std::string root_directory = "root";

    // Create the file path
    std::string file_path = root_directory + request.get_uri();

    std::cout << request.get_body().data() << std::endl;

    // Open the file for writing
    std::ofstream file(file_path, std::ios::binary | std::ios::trunc);

    if (file) 
    {   
        // Write the content from the request body to the file
        file.write(request.get_body().data(), request.get_body().size());
        if (file.bad()) {
            std::cerr << "Error: could not write data\n";
            file.close();
            return -1;
        }
        file.close();

        // Send the HTTP response header
        std::string response_header = "HTTP/1.1 201 Created\r\n";
        response_header += "\r\n";

        if (send(client_socket, response_header.c_str(), response_header.size(), 0) == -1) {
            std::cerr << "Error: could not send data\n";
            return -1;
        }
        return 0;
    } 
    else // If file not found
    {
        return send_error_response(client_socket, 500, "Internal Server Error");
    }
}

int handle_delete_request(int client_socket, const Request& request)
{
    // Set the root directory for serving files
    const std::string root_directory = "root";

    // Create the file path
    std::string file_path = root_directory + request.get_uri();

    // Try to delete the file
    if (std::remove(file_path.c_str()) == 0) 
    {
        // Send success response
        std::string response_header = "HTTP/1.1 204 No Content\r\n";
        response_header += "\r\n";
        if (send(client_socket, response_header.c_str(), response_header.size(), 0) == -1) {
            std::cerr << "Error: could not send data\n";
            return -1;
        }
    }
    else 
    {
        // File not found or couldn't be deleted
        return send_error_response(client_socket, 404, "Not Found");
    }
    return 0;
}

int send_error_response(int client_socket, int status_code, const std::string& status_message)
{
    // Create the error page content
    std::string error_page = "<html><head><title>Error " + std::to_string(status_code) + "</title></head>";
    error_page += "<body><h1>Error " + std::to_string(status_code) + ": " + status_message + "</h1></body></html>";

    // Send the HTTP response header
    std::string response_header = "HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n";
    response_header += "Content-Type: text/html\r\n";
    response_header += "Content-Length: " + std::to_string(error_page.size()) + "\r\n";
    response_header += "\r\n";

    if (send(client_socket, response_header.c_str(), response_header.size(), 0) == -1) {
        std::cerr << "Error: could not send data\n";
        return -1;
    }

    // Send the error page content
    if (send(client_socket, error_page.data(), error_page.size(), 0) == -1) {
        std::cerr << "Error: could not send data\n";
        return -1;
    }
    return 0;
}
