#include "methods.hpp"

int handle_get_request(int client_socket, const Request& request, std::string root)
{
    const std::string root_directory = root; // le root sera different DONC A CHANGER

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

int handle_post_request(int client_socket, const Request& request, std::string root)
{
    // Set the root directory for serving files
    const std::string root_directory = root;

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

int handle_delete_request(int client_socket, const Request& request, std::string root)
{
    // Set the root directory for serving files
    const std::string root_directory = root;

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
    std::string error_page;
    std::string response_header = "HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n";
    response_header += "Content-Type: text/html\r\n";

    // Default styles for all error pages
    std::string default_style = "<style>"
                            "body {"
                                "font-family: Arial, sans-serif;"
                                "margin: 0;"
                                "padding: 0;"
                                "background-color: #f0f4f5;"
                                "display: flex;"
                                "justify-content: center;"
                                "align-items: center;"
                                "height: 100vh;"
                            "}"
                            ".container {"
                                "text-align: center;"
                            "}"
                            ".error {"
                                "font-size: 72px;"
                            "}"
                            ".message {"
                                "font-size: 24px;"
                            "}"
                        "</style>";

    std::string error_color;
    std::string message_color;

    switch (status_code) 
    {
        case 400:
            error_color = "#ff0000";  // Red
            message_color = "#ff0000";  // Red
            // Custom error page for 400
            error_page = "<html><head><title>Error 400</title>" + default_style + "<style>.error,.message { color: " + error_color + "; }</style></head>";
            error_page += "<body><div class='container'><h1 class='error'>Error 400</h1><p class='message'>The Request cannot be processed by the server</p></div></body></html>";
            break;

        case 404:
            error_color = "#ffa500";  // Orange
            message_color = "#ffa500";  // Orange
            // Custom error page for 404
            error_page = "<html><head><title>Error 404</title>" + default_style + "<style>.error,.message { color: " + error_color + "; }</style></head>";
            error_page += "<body><div class='container'><h1 class='error'>Error 404</h1><p class='message'>The page you requested could not be found.</p></div></body></html>";
            break;

        case 405:
            error_color = "#eae22f";  // Yellow
            message_color = "#eae22f";  // Yellow
            // Custom error page for 405
            error_page = "<html><head><title>Error 405</title>" + default_style + "<style>.error,.message { color: " + error_color + "; }</style></head>";
            error_page += "<body><div class='container'><h1 class='error'>Error 405</h1><p class='message'>Method Not Allowed.</p></div></body></html>";
            break;

        case 500:
            error_color = "#008000";  // Green
            message_color = "#008000";  // Green
            // Custom error page for 500
            error_page = "<html><head><title>Error 500</title>" + default_style + "<style>.error,.message { color: " + error_color + "; }</style></head>";
            error_page += "<body><div class='container'><h1 class='error'>Error 500</h1><p class='message'>Internal Server Error.</p></div></body></html>";
            break;

        default:
            error_color = "#154a6e";  // Default color
            message_color = "#154a6e";  // Default color
            error_page = "<html><head><title>Error " + std::to_string(status_code) + "</title>" + default_style + "<style>.error,.message { color: " + error_color + "; }</style></head>";
            error_page += "<body><div class='container'><h1 class='error'>Error " + std::to_string(status_code) + "</h1><p class='message'>" + status_message + "</p></div></body></html>";
            break;
    }

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
