#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <sys/socket.h>

class Request 
{
    public:

        Request();
        Request(const std::string& request_str);
        
        int                                 parse(char *buffer, size_t length);
        const                               std::string& get_method() const{return this->method;};
        const                               std::string& get_uri() const{return this->uri;};
        const                               std::string& get_http_version() const{return this->http_version;};
        const                               std::map<std::string, std::string>& get_headers() const{return this->headers;};
        const                               std::string& get_body() const{return this->body;};
        bool                                is_cgi() const;

    private:
        std::string                         method;
        std::string                         uri;
        std::string                         http_version;
        std::map<std::string, std::string>  headers;
        std::string                         body;
};

#endif
