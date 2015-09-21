#ifndef RESTPARAMETERS_H
#define RESTPARAMETERS_H

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string,std::string> StringMap;

class RESTParameters{
private:
    StringMap mParameters;

public:
    RESTParameters(const std::string &params, const StringMap &registeredParams);
    ~RESTParameters();

    std::string getParam(std::string key);

};

#endif

