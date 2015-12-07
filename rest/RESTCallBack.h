#pragma once
#include <unordered_map>
#include "json/JSON.h"
#include "RESTParameters.h"
#include "RESTEngine.h"
#include <functional>
#include <regex>

struct RESTContext
{
    Dumais::JSON::JSON& returnData;
    RESTParameters* params;
    const std::string& data;
    std::smatch& matches;
    RESTEngine::ResponseCode &responseCode;
    std::shared_ptr<void> userData;
};

class RESTCallBack
{
private:
    std::function<void(RESTContext*)> mCallback;
    ParamMap mParamList;
    std::string mDescription;
public:
    template<class T> RESTCallBack(T* obj,
        void(T::*func)(RESTContext*),
        const std::string& description)
    {
        mDescription = description;
        mCallback = std::bind(func, obj, std::placeholders::_1);
    }
    ~RESTCallBack();

    /*
     * add a Parameter to this callback
     * @param : parameter name
     * @description : description of the parameter
     * @required : is this parameter required for the callback to be triggered
     * @type: default is "string"
     * @location: in "query" (default) or in "path"
     */
    void addParam(const std::string &param, const std::string &description,
                  bool required = true, const std::string &type="string",
                  const std::string &location="query", const std::string &defaultValue="");
    void getDescription(Dumais::JSON::JSON& json);
    void getSwaggerDescription(Dumais::JSON::JSON& json)
    {
        json.addValue(mDescription,"summary");
        json.addValue(mDescription,"description");
        if (mParamList.empty())
            return;
        Dumais::JSON::JSON& params = json.addList("parameters");
        for (ParamMap::iterator it = mParamList.begin(); it!=mParamList.end(); it++)
        {
            Dumais::JSON::JSON& j = params.addObject("param");
            j.addValue(it->first,"name");
            j.addValue(it->second.mDescription, "description");
            j.addValue(it->second.mType, "type");
            j.addValue(it->second.mLocation, "in");
            j.addValue(it->second.mRequired, "required");
            if (!it->second.mDefaultValue.empty())
                j.addValue(it->second.mDefaultValue, "default");
        }
    }
    RESTEngine::ResponseCode call(Dumais::JSON::JSON& json, const std::string& paramString,
                                  const std::string& data, std::smatch&, std::shared_ptr<void> userData = NULL);
};
