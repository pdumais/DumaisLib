#pragma once
#include <unordered_map>
#include "JSON.h"
#include "RESTParameters.h"
#include <functional>

enum class RESTMethod
{
    POST,
    GET,
    PUT,
    DELETE
};


class RESTCallBack
{
private:
    std::function<void(Dumais::JSON::JSON&, RESTParameters*, const std::string&)> mCallback;
    StringMap mParamList;
    std::string mDescription;
public:
    template<class T> RESTCallBack(T* obj,
        void(T::*func)(Dumais::JSON::JSON&, RESTParameters*, const std::string&),
        const std::string& description)
    {
        mDescription = description;
        mCallback = std::bind(func,obj,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
    }
    ~RESTCallBack();

    void addParam(std::string param, std::string description);
    void getDescription(Dumais::JSON::JSON& json);
    void call(Dumais::JSON::JSON& json, const std::string& paramString, const std::string& data);
};

