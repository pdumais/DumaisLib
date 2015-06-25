#include "RESTCallBack.h"

RESTCallBack::~RESTCallBack()
{
}

void RESTCallBack::addParam(std::string param, std::string description)
{
    mParamList[param] = description;
}

void RESTCallBack::getDescription(Dumais::JSON::JSON& json)
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

void RESTCallBack::call(Dumais::JSON::JSON& json, const std::string& paramString, const std::string& dataString, std::smatch& matches)
{
    RESTParameters params(paramString,mParamList);
    RESTContext context ={
        json,
        &params,
        dataString,
        matches
    };
    if (mCallback) mCallback(context);
}

