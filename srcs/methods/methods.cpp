#include "methods.hpp"

std::pair<bool, Location>   check_location(ServInfo& current_port, const std::string& request_location) 
{
    for (size_t i = 0; i < current_port.getLocation().size(); i++) {
        if (current_port.getLocation()[i].getPath() == request_location) {
            return std::make_pair(true, current_port.getLocation()[i]);
        }
    }
    // Return a default Location if not found
    return std::make_pair(false, Location());
}


/****************************/
/*           GET            */
/****************************/

// function to get file extension
std::string get_extension(const std::string& file_path) 
{
    size_t pos = file_path.find_last_of('.');
    if (pos != std::string::npos) {
        return file_path.substr(pos);
    } else {
        return "";
    }
}

std::string get_mime_type(const std::string& extension) 
{
    // Here you could add more mappings for different file types as per your needs
    if (extension == ".html") return "text/html";
    else if (extension == ".css") return "text/css";
    else if (extension == ".js") return "application/javascript";
    else if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    else if (extension == ".png") return "image/png";
    else if (extension == ".gif") return "image/gif";
    else return "text/plain"; // default to plain text
}

// This function is used to send the content of a directory as an HTML list.
int handle_directory_request(Socket method_socket, const std::string& directory_path, const std::string& root) 
{
    std::string response_body = "<html><body><ul>";

    DIR* dirp = opendir(directory_path.c_str());
    if (dirp == NULL) 
    {
        std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
        return -1;
    }

    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
        std::string filename = dp->d_name;
        
        // Skip directories that start with a dot
        if (filename[0] == '.')
            continue;

        // Add the relative path from the root to the filename.
        std::string filepath = directory_path.substr(root.length()) + "/" + filename; 
        response_body += "<li><a href=\"" + filepath + "\">" + filename + "</a></li>";
    }

    closedir(dirp);

    response_body += "</ul></body></html>";

    // Send the HTTP response header
    std::string response_header = "HTTP/1.1 200 OK\r\n";
    response_header += "Content-Type: text/html\r\n";
    response_header += "Content-Length: " + std::to_string(response_body.size()) + "\r\n";
    response_header += "\r\n";
    std::string response = response_header + response_body;

    return (method_socket.send(response.c_str(), response.size()));
}

int handle_get_request(Socket method_socket, const Request& request, ServInfo port)
{
    std::string root = port.getRoot();
    std::pair<bool, Location> location_check = check_location(port, request.get_uri());
    if (location_check.first)
    {
        // Response header for a redirection
        std::string response_header = "HTTP/1.1 301 Moved Permanently\r\n";
        response_header += "Location: " + location_check.second.getRedir() + "\r\n";
        response_header += "\r\n";

        return(method_socket.send(response_header.c_str(), response_header.size()));
    }

    // Create the file path
    std::string file_path = root + request.get_uri();

    struct stat path_stat;
    stat(file_path.c_str(), &path_stat);
    int is_directory = S_ISDIR(path_stat.st_mode);

    // If the requested URI is a directory
    if (is_directory) 
    {
        bool    dir_listing = port.getDirListing();
        if (location_check.first)
            dir_listing = location_check.second.getDirListing();
        // Check if directory listing is enabled
        if (dir_listing && file_path != port.getRoot() + "/")
        {
            // Handle directory request
            return handle_directory_request(method_socket, file_path, root);
        }
        else if (file_path == port.getRoot() + "/")
        {
            // Default to "index.html"
            file_path += port.getIndex();
        }
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
        response_header += "Content-Type: " + get_mime_type(get_extension(file_path)) + "\r\n";
        response_header += "Content-Length: " + std::to_string(file_content.size()) + "\r\n";
        response_header += "\r\n";

        std::string response = response_header + file_content;

        return (method_socket.send(response.c_str(), response.size()));
    } 
    else // If file not found
    {
        send_error_response(method_socket.get_fd(), 404, "Not Found", port);
        return -1;
    }
    return 0;
}
/****************************/
/*           POST           */
/****************************/

int handle_post_request(Socket method_socket, const Request& request, ServInfo port)
{
    // Get the root directory for serving files
    std::string root_directory = port.getRoot();

    // Create the file path
    std::string file_path = root_directory + request.get_uri();

    // Check if the file already exists
    struct stat buffer;
    if (stat(file_path.c_str(), &buffer) == 0) 
    {
        // File already exists, return an error response
        send_error_response(method_socket.get_fd(), 409, "Conflict: File already exists", port);
        return -1;
    }

    // Open the file for writing
    std::ofstream file(file_path, std::ios::binary | std::ios::trunc);

    if (file) 
    {
        // Write the content from the request body to the file
        file.write(request.get_body().data(), request.get_body().size());
        if (file.bad()) 
        {
            send_error_response(method_socket.get_fd(), 500, "Internal Server Error: Could not write data", port);
            std::cerr << "Error: could not write data\n";
            file.close();
            return -1;
        }
        file.close();

        // Send the HTTP response header
        std::string response_header = "HTTP/1.1 201 Created\r\n";
        response_header += "Content-Type: text/plain\r\n";
        std::string response_body = "File successfully created.\r\n";
        response_header += "Content-Length: " + std::to_string(response_body.size()) + "\r\n";
        response_header += "\r\n";
        response_header += response_body;

        return (method_socket.send(response_header.c_str(), response_header.size()));
    } 
    else 
    {
        // File failed to open, return error
        send_error_response(method_socket.get_fd(), 500, "Internal Server Error: Could not open file", port);
        return -1;
    }
}

/****************************/
/*          DELETE          */
/****************************/

int handle_delete_request(Socket method_socket, const Request& request, ServInfo port)
{
    std::string root = port.getRoot();
    std::pair<bool, Location> location_check = check_location(port, request.get_uri());
    if (location_check.first)
        root = location_check.second.getRoot();
    
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
        return (method_socket.send(response_header.c_str(), response_header.size()));
    }
    else 
    {
        // File not found or couldn't be deleted
        return send_error_response(method_socket.get_fd(), 404, "Not Found", port);
    }
    return 0;
}

int send_error_response(int client_socket, int status_code, const std::string& status_message, ServInfo port)
{
    std::string error_page;
    std::string response_header = "HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n";
    response_header += "Content-Type: text/html\r\n";

    std::string errorPagePath = port.getErrors();

    if (!errorPagePath.empty()) 
    {
        
        // Read the file content
        std::ifstream file(errorPagePath, std::ios::binary);

        if (file) 
        {
            // Read the file content into a string
            std::ostringstream file_stream;
            file_stream << file.rdbuf();
            error_page = file_stream.str();

            response_header += "Content-Length: " + std::to_string(error_page.size()) + "\r\n";
            response_header += "\r\n";
            std::string response = response_header + error_page.data();
            return (send(client_socket, response.c_str(), response.size(), 0)); 
            
            return 0;
        } 
        else // If custom error file not found, fall back to default error message
        {
            std::cerr << "Error: could not open custom error page\n";
        }
    }

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
            break;
        case 404:
            error_color = "#ffa500";  // Orange
            message_color = "#ffa500";  // Orange
            break;
        case 405:
            error_color = "#eae22f";  // Yellow
            message_color = "#eae22f";  // Yellow
            break;
        case 500:
            error_color = "#008000";  // Green
            message_color = "#008000";  // Green
            break;
        default:
            error_color = "#154a6e";  // Default color
            message_color = "#154a6e";  // Default color
            break;
    }

    // The error page now uses the status_message parameter
    error_page = "<html><head><title>Error " + std::to_string(status_code) + "</title>" + default_style + "<style>.error,.message { color: " + error_color + "; }</style></head>";
    error_page += "<body><div class='container'><h1 class='error'>Error " + std::to_string(status_code) + "</h1><p class='message'>" + status_message + "</p></div></body></html>";

    response_header += "Content-Length: " + std::to_string(error_page.size()) + "\r\n";
    response_header += "\r\n";
    std::string response = response_header + error_page.data();

    ssize_t returned = send(client_socket, response.c_str(), response.size(), 0); 
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
