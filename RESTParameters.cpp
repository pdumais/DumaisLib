#include "RESTParameters.h"

RESTParameters::RESTParameters(std::string params, StringMap registeredParams)
{
    std::string key;
    std::string value;
    bool keyFlag = true;
    int n=0;
    while (params[n]!=' ' && n<params.size())
    {
        if (params[n]=='=')
        {
            keyFlag = false;
        }
        else if (params[n]=='&' || params[n]==' ' || params[n]=='\r' || params[n]=='\n')
        {
            if (registeredParams.find(key)!=registeredParams.end()) mParameters[key]=value;
            keyFlag = true;
            key="";
            value="";
        } 
        else
        {
            if (keyFlag)
            {
                key+=params[n];
            }
            else
            {
                value+=params[n];
            }
        }

        n++;
        if (params[n]==' ' || params[n]=='\r' || params[n]=='\n' || n==params.size())
        {
            if (registeredParams.find(key)!=registeredParams.end()) mParameters[key]=value;
            key="";
            value="";
        }
    }
}

RESTParameters::~RESTParameters(){
}

std::string RESTParameters::getParam(std::string key)
{
    if (mParameters.find(key)!=mParameters.end()) return mParameters[key];

    return "";
}
