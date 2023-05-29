#ifndef SERVINFO_HPP
#define SERVINFO_HPP

#include <iostream>
#include <vector>
#include <map>
#include "Location.hpp"

class Location;

class ServInfo {

public:
    ServInfo(): _port(0), _body_size(0) {};
    ~ServInfo() {};

    /* Getters */

    int getPort(void) const { return _port; };
    std::string getName(void) const { return _name; };
    std::vector<std::string> getMethods(void) const { return _methods; };
    std::string getRoot(void) const { return _root_dir; };
    bool getAutoindex(void) const { return _autoindex; };
    size_t getBody_size(void) const { return _body_size; };
    std::map<int, std::string> getErrors(void) const { return _error_pages; };
    std::vector<Location> getLocation(void) const { return _location; };
    bool getDirListing(void) const { return _dir_listing; };
    std::string getIndex(void) const { return _index; };

    /* Setters */

    void setPort(int p) { _port = p; };
    void setName(std::string n) { _name = n; };
    void setMethod(std::string m) { _methods.push_back(m); };
    void setRoot(std::string r) { _root_dir = r; };
    void setAutoindex(bool a) { _autoindex = a; };
    void setBody_size(size_t b) { _body_size = b; };
    void setErrors(int code, std::string error) { _error_pages.insert(std::pair<int, std::string>(code, error)); };
    void setLocation(Location l) { _location.push_back(l); };
    void setDirListing(bool d) { _dir_listing = d; };
    void setIndex(std::string i) { _index = i; };

protected:
    int _port;
    std::string _name;
    std::vector<std::string> _methods;
    std::string _root_dir;
    bool _autoindex;
    size_t _body_size;
    std::string _index;
    std::map<int, std::string> _error_pages;
    std::vector<Location> _location;
    bool _dir_listing;

};

#endif
