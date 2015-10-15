#ifndef RESTPARAMETERS_H
#define RESTPARAMETERS_H

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string,std::string> StringMap;

class RESTParameter {
public:
    /*
     * Create a new  RESTParameter
     */
    RESTParameter(const std::string &name, const std::string &desc,
                  bool required=true, const std::string &type="string",
                  const std::string &location="query", const std::string &defaultValue=""):
    mName(name), mDescription(desc), mLocation(location), mDefaultValue(defaultValue), mType(type), mRequired(required)
    {
    }
    const std::string mName;
    const std::string mDescription;
    const std::string mLocation;
    const std::string mDefaultValue;
    const std::string mType;
    const bool mRequired;
    ~RESTParameter() {}
private:
};

typedef std::unordered_map<std::string, RESTParameter> ParamMap;

class RESTParameters {
private:
    StringMap mParametersValues;

public:
    RESTParameters(const std::string &params, const ParamMap &registeredParams);
    ~RESTParameters();

    std::string getParam(std::string key);

};

#endif

