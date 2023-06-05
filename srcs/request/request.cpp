#include "request.hpp"


Request::Request() {}

Request::Request(const std::string& request_str) 
{
    std::istringstream request_stream(request_str);
    std::string line;
    std::getline(request_stream, line);
    std::istringstream first_line(line);
    first_line >> method >> uri >> http_version;

    // Read headers
    while (std::getline(request_stream, line) && line != "\r") 
    {
        std::size_t separator = line.find(':');
        if (separator != std::string::npos) 
        {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 1);
            headers[key] = value;
        }
    }
}

int Request::parse(char *buffer, size_t length)
{
    // Check for end of headers (double CRLF)
    const char* end_headers = "\r\n\r\n";
    if (std::search(buffer, buffer + length, end_headers, end_headers + 4) == buffer + length)
    {
        std::cerr << "Incomplete HTTP request\n";
        return -1;  // Incomplete request
    }
    
    std::istringstream request_stream(std::string(buffer, length));
    std::string line;

    // Parse request line
    std::getline(request_stream, line);
    std::istringstream first_line(line);
    first_line >> method >> uri >> http_version;
    

    // Parse headers
    while (std::getline(request_stream, line) && line != "\r") 
    {
        std::size_t separator = line.find(':');
        if (separator != std::string::npos) {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 2);

            // Remove leading and trailing whitespaces
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            headers[key] = value;
        }
    }

    // Extract boundary from Content-Type header
    std::string boundary;
    if (headers.count("Content-Type")) 
    {
        size_t boundary_pos = headers["Content-Type"].find("boundary=");
        if (boundary_pos != std::string::npos) {
            boundary = headers["Content-Type"].substr(boundary_pos + 9);
        }
    }

    // Read body if any
    std::streamsize content_length = 0;
    if (headers.count("Content-Length")) 
    {
        content_length = std::stol(headers["Content-Length"]);
    }
    body.resize(content_length);
   request_stream.read(&body[0], content_length);
   if (content_length == -1) 
   {
       std::cerr << "Error: could not read data\n";
       return -1;
   }
   return 0;
}

bool Request::is_cgi() const 
{
    std::string path = get_uri();
    size_t pos = path.rfind(".");
    if (pos == std::string::npos) 
    {
        return false;
    }

    std::string ext = path.substr(pos);
    return ext == ".cgi" || ext == ".py" || ext == ".pl";
}

