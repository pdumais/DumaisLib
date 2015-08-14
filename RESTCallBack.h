#pragma once
#include <unordered_map>
#include "JSON.h"
#include "RESTParameters.h"
#include <functional>
#include <regex>

//enum class RESTMethod : "enum class" not supported by MSVC2010
enum RESTMethod
{
    POST,
    GET,
    PUT,
    DELETE
};

struct RESTContext
{
    Dumais::JSON::JSON& returnData;
    RESTParameters* params;
    const std::string& data;
    std::smatch& matches;
};

class RESTCallBack
{
private:
    std::function<void(RESTContext)> mCallback;
    StringMap mParamList;
    std::string mDescription;
public:
    template<class T> RESTCallBack(T* obj,
        void(T::*func)(RESTContext),
        const std::string& description)
    {
        mDescription = description;
        mCallback = std::bind(func,obj,std::placeholders::_1);
    }
    ~RESTCallBack();

    void addParam(std::string param, std::string description);
    void getDescription(Dumais::JSON::JSON& json);
    void call(Dumais::JSON::JSON& json, const std::string& paramString, const std::string& data, std::smatch&);
};

