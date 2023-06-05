#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <utility>

class Location {

public:
    Location() {};
    ~Location() {};

    Location& operator=(const Location & other) {
        _redir = other._redir;
        _dir = other._dir;
        _methods = other._methods;
        _root = other._root;
        _index = other._index;
        _dir_listing = other._dir_listing;
        _path = other._path;
        return *this;
    }

    std::string getRedir(void) const { return _redir; };
    std::string getDir(void) const { return _dir; };
    std::vector<std::string> getMethods(void) const { return _methods; };
    std::string getRoot(void) const { return _root; };
    std::string getIndex(void) const { return _index; };
    bool getDirListing(void) const { return _dir_listing; };
    std::string getPath(void) const { return _path; };

    void setRedir(std::string r) { _redir = r; };
    void setDir(std::string d) { _dir = d; };
    void setMethod(std::string m) { _methods.push_back(m); };
    void setRoot(std::string r) { _root = r; };
    void setIndex(std::string i) { _index = i; };
    void setDirListing(bool d) { _dir_listing = d; };
    void setPath(std::string p) { _path = p; };

private:
    std::string _path;
    std::string _redir;
    std::string _dir;
    std::vector<std::string> _methods;
    std::string _root;
    std::string _index;
    bool _dir_listing;
};

#endif
