#include "RESTParameters.h"

RESTParameters::RESTParameters(std::string params, StringMap registeredParams)
{
    std::string key;
    std::string value;
    bool keyFlag = true;
    int n=0;
    // test first if n is legal before reading params to avoid crash on MSVC
    while (n<params.size() && params[n]!=' ')
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
        if (n==params.size() || params[n]==' ' || params[n]=='\r' || params[n]=='\n')
        {
            if (registeredParams.find(key)!=registeredParams.end()) mParameters[key]=value;
            key="";
            value="";
            break;
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
