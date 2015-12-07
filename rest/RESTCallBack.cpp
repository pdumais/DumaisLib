#include <assert.h>
#include "RESTCallBack.h"

RESTCallBack::~RESTCallBack()
{
}

void RESTCallBack::addParam(const std::string &paramName, const std::string &description,
           bool required, const std::string &type, const std::string &location, const std::string &defaultValue)
{
    mParamList.insert(std::pair<std::string, RESTParameter>(paramName,
                         RESTParameter(paramName, description, required, type, location, defaultValue)) );
}

void RESTCallBack::getDescription(Dumais::JSON::JSON& json)
{
    json.addValue(mDescription,"description");
    Dumais::JSON::JSON& params = json.addList("params");
    for (ParamMap::const_iterator it = mParamList.begin(); it!=mParamList.end(); it++)
    {
        Dumais::JSON::JSON& j = params.addObject("param");
        j.addValue(it->first,"name");
        j.addValue(it->second.mDescription, "description");
    }
}

RESTEngine::ResponseCode RESTCallBack::call(Dumais::JSON::JSON& json, const std::string& paramString, const std::string& dataString, std::smatch& matches)
{
    RESTParameters params(paramString, mParamList);
    RESTEngine::ResponseCode responseCode = RESTEngine::ResponseCode::OK;
    RESTContext context ={
        json,
        &params,
        dataString,
        matches,
        responseCode
    };
    assert(mCallback);
    if (mCallback)
        mCallback(&context);
    return responseCode;
}
