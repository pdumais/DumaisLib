#pragma once
#include <unordered_map>
#include "JSON.h"
#include "RESTParameters.h"

enum class RESTMethod
{
    POST,
    GET,
    PUT,
    DELETE
};


class IRESTCallBack
{
private:
public:
    virtual void call(Dumais::JSON::JSON& json, const std::string& params, const std::string& data)=0;
    virtual void getDescription(Dumais::JSON::JSON& json) = 0;
};



template <class T>
class RESTCallBack: public IRESTCallBack{
private:
    void (T::*mpCallback)(Dumais::JSON::JSON&,RESTParameters* p, const std::string& data);
    T *mpObject;
    StringMap mParamList;
    std::string mDescription;
public:
    RESTCallBack(T *obj, void (T::*pCB)(Dumais::JSON::JSON&,RESTParameters*, const std::string& data),std::string desc)
    {
        mDescription = desc;
        mpObject = obj;
        mpCallback = pCB;
    }

    ~RESTCallBack(){};

    void addParam(std::string param, std::string description)
    {
       mParamList[param] = description;
    }

    void getDescription(Dumais::JSON::JSON& json)
    {
        json.addValue(mDescription,"description");
        json.addList("params");
        for (StringMap::iterator it = mParamList.begin();it!=mParamList.end();it++)
        {
            Dumais::JSON::JSON& j = json["params"].addObject("param");
            j.addValue(it->first,"name");
            j.addValue(it->second,"description");
        }
    }

    void call(Dumais::JSON::JSON& json, const std::string& paramString, const std::string& data)
    {
        RESTParameters params(paramString,mParamList);
        (mpObject->*mpCallback)(json,&params, data);
    }
};

