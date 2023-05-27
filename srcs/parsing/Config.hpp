#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <map>
#include <set>
#include "Location.hpp"
#include "ServInfo.hpp"

class Config: public ServInfo {

public:
    Config(const std::string& path){
        if (path.substr(path.find_last_of('.') + 1) != "conf")
            errorExit("Bad file extension");
        std::ifstream config_file(path.c_str());
        if (!config_file){
            std::cout << "Cannot open config file: " << path << std::endl;
            exit(1);
        }
        std::string line;
        ServInfo current_server;
        Location current_location;
        bool in_location = false;
        bool has_name = false;
        bool has_port = false;
        _totalServs = 0;

        while (std::getline(config_file, line)) {
            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword == "server") {
                if (in_location) {
                    current_server.setLocation(current_location);
                    current_location = Location();
                    in_location = false;
                    has_name = false;
                    has_port = false;
                }
                if (!current_server.getName().empty()) {
                    _totalServs++;
                    checkMandatory(current_server);
                    _configServ.push_back(current_server);
                    current_server = ServInfo();
                }
            }
            else if (keyword == "location") {
                handleLocation(in_location, current_server, current_location, ss);
            }
            else{
                handleKeywords(keyword, in_location, current_server, current_location, ss, has_name, has_port);
            }
        }

        addLastServ(in_location, current_server, current_location, has_name, has_port);
        config_file.close();
    }
    ~Config(){};

    std::vector<ServInfo> getListOfServ(void) const { return _configServ; };

    void handleLocation(bool& in_location, ServInfo& current_server, Location& current_location, std::stringstream& ss) {
        if (in_location) {
            checkLocation(current_location);
            std::string str = current_location.getPath();
            std::string root = current_location.getRoot();
            current_location.setPath(root + str);
            current_server.setLocation(current_location);
            current_location = Location();
        }
        in_location = true;
        std::string path;
        ss >> path;
        current_location.setPath(path);
    }

    void handleKeywords(const std::string& keyword, bool& in_location, ServInfo& current_server, Location& current_location, std::stringstream& ss, bool& has_name, bool& has_port) {
        if (keyword == "server_name" && !has_name) {
            std::string name;
            ss >> name;
            if (name.empty() || !current_server.getName().empty())
                errorExit("Error: wrong server name");
            current_server.setName(name);
            has_name = true;
        }
        else if (keyword == "listen" && !has_port) {
            int port;
            ss >> port;
            if ((port < 1024 || port > 65535)  || current_server.getPort())
                errorExit("Error: wrong port");
            current_server.setPort(port);
            has_port = true;
        }
        else if (keyword == "root") {
            std::string root;
            ss >> root;
            std::ifstream path(root.c_str());
            if (!path)
                errorExit("Error: cannot access root path");
            if (in_location)
                current_location.setRoot(root);
            else
                current_server.setRoot(root);
        }
        else if (keyword == "index") {
            struct stat buffer;
            std::string path;
            std::string index;
            ss >> index;
            if (current_server.getRoot().empty())
                errorExit("Error: no root path to access index");
            path = current_server.getRoot() + index;
            if (stat(path.c_str(), &buffer) != 0)
                errorExit("Error: Index file does not exist");
            if (in_location)
                current_location.setIndex(index);
            else
                current_server.setIndex(index);
        }
        else if (keyword == "allowed_methods") {
            std::string method;
            while (ss >> method) {
                if (!checkValidMethod(method)) {
                    std::cerr << "Error: Method " << method << " is invalid or does not exists\n";
                    exit(1);
                }
                if (in_location)
                    current_location.setMethod(method);
                else
                    current_server.setMethod(method);
            }
        }
        else if (keyword == "dir_listing") {
            std::string dir_listing;
            ss >> dir_listing;
            if (in_location)
                current_location.setDirListing(true);
            else
                current_server.setDirListing(true);
        }
        else if (keyword == "client_max_body_size") {
            size_t body_size;
            ss >> body_size;
            current_server.setBody_size(body_size);
        }
        else if (keyword == "error_page") {
            int code;
            std::string path;
            struct stat info;
            ss >> code >> path;
            if (stat(path.c_str(), &info) != 0)
                errorExit("Error: error_page path does not exist");
            current_server.setErrors(code, path);
        }
        else if (keyword == "redir") {
            std::string redir;
            ss >> redir;
            current_location.setRedir(redir);
        }
        else if (keyword == "dir") {
            std::string dir;
            struct stat info;
            ss >> dir;
            if (stat(dir.c_str(), &info) != 0)
                errorExit("Error: dir path does not exist");
            current_location.setDir(dir);
        }
    }

    void addLastServ(bool& in_location, ServInfo& current_server, Location& current_location, bool& has_name, bool& has_port) {
        if (in_location) {
            checkLocation(current_location);
            current_server.setLocation(current_location);
            current_location = Location();
            in_location = false;
            has_name = false;
            has_port = false;
        }
        if (current_server.getName().empty())
            checkMandatory(current_server);
        if (!current_server.getName().empty()) {
            _totalServs++;
            checkMandatory(current_server);
            _configServ.push_back(current_server);
            current_server = ServInfo();
        }
    }

    bool checkValidMethod(std::string& method) {
        std::set<std::string> methods;
        methods.insert("GET");
        methods.insert("DELETE");
        methods.insert("POST");

        return methods.find(method) != methods.end();
    }

    void checkMandatory(ServInfo& current_server) {
        if (current_server.getName().empty())
            errorExit("Error: no server name");
        if (!current_server.getPort())
            errorExit("Error: no port");
        if (current_server.getErrors().empty()) {
            std::cerr << "Error: no error pages found" << std::endl;
            exit(1);
        }
        if (current_server.getRoot().empty())
            errorExit("Error: no root directory found");
        if (current_server.getIndex().empty())
            errorExit("Error: no index file found");
        if (current_server.getMethod().empty())
            errorExit("Error: no methods found");
    }

    void checkLocation(Location current_location) {
        if (current_location.getRoot().empty())
            errorExit("Error: no root directory for Location found");
        if (current_location.getIndex().empty())
            errorExit("Error: no index file for Location found");
    }

    int getTotalServs(void) const { return _totalServs; };
    std::vector<ServInfo> getConfig(void) { return _configServ; };
    bool is_allowed_method(std::string serv_name, std::string method) {
        for (size_t i = 0; i < _configServ.size(); i++) {
            if (serv_name.compare(_configServ[i].getName()) == 0) {
                for (size_t j = 0; j < _configServ[i].getMethod().size(); j++) {
                    if (_configServ[i].getMethod()[j] == method)
                        return true;
                }
            }
        }
        return false;
    }

private:
    Config(){};
    std::vector<ServInfo> _configServ;
    int _totalServs;
    std::string dir;


    void errorExit(std::string str) {
        std::cerr << str << std::endl;
        exit(1);
    }
};

#endif
