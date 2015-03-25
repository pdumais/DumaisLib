#ifndef RESTPARAMETERS_H
#define RESTPARAMETERS_H

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string,std::string> StringMap;

class RESTParameters{
private:
    StringMap mParameters;

public:
	RESTParameters(std::string params, StringMap registeredParams);
	~RESTParameters();

    std::string getParam(std::string key);

};

#endif

