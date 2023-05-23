#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <unistd.h>
#include "../socket/Socket.hpp"
#include "../request/request.hpp"

class CGI 
{
    public:
        CGI(const std::string& script_path, const std::string& query_string, const Request& request, int port);
        ~CGI();

        std::string run_cgi_script();

    private:
        const std::string&  _script_path;
        const std::string&  _query_string;
        const Request&      _request;
        const int           _port;

};

#endif
