#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <unistd.h>
#include "../socket/Socket.hpp"
#include "../request/request.hpp"
#include "../parsing/ServInfo.hpp"

class CGI 
{
    public:
        CGI(const std::string& script_path, const std::string& query_string, const Request& request, std::vector<ServInfo> ports, int client_fd);
        ~CGI();

        int run_cgi_script(std::string &response);

    private:
        const std::string&  _script_path;
        const std::string&  _query_string;
        const Request&      _request;
        ServInfo            _port_info;

};

#endif
