#include <assert.h>
#include <iostream>
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

RESTEngine::ResponseCode RESTCallBack::call(Dumais::JSON::JSON& json, const std::string& paramString,
                                            const std::string& dataString, std::smatch& matches,
                                            std::shared_ptr<void> userData)
{
    RESTParameters params(paramString, mParamList);
    // Let's check required params
    for (ParamMap::const_iterator it = mParamList.begin(); it != mParamList.end(); ++it) {
        // todo later : handle/check in "path" parameters: http://server:port/param1/param2
        if (it->second.mRequired && (it->second.mLocation == "query") ) {
            if (params.getParam(it->first).empty()) {
                std::cerr << "Missing mandatory parameter " <<  it->first << std::endl;
                // todo : replace empty parameter value by default parameter value
                json.addValue("Missing required parameter " + it->first, "error");
                return RESTEngine::ResponseCode::BadRequest;
            }
        }
    }

    RESTEngine::ResponseCode responseCode = RESTEngine::ResponseCode::OK;
    RESTContext context ={
        json,
        &params,
        dataString,
        matches,
        responseCode,
        userData
    };
    assert(mCallback);
    if (mCallback)
        mCallback(&context);
    return responseCode;
}
