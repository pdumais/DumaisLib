#include "RESTEngine.h"
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <regex>

RESTEngine::RESTEngine(){
}

RESTEngine::~RESTEngine()
{
    for (CallbackMap::iterator it = mCallbacks.begin(); it != mCallbacks.end(); it++)
    {    
        for (std::list<ResourceIdentifier>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            delete it2->mpCallback;
        }
    }
}

void RESTEngine::addCallBack(std::string uri, std::string method, RESTCallBack* p)
{
    ResourceIdentifier ri;
    ri.regex.assign(uri);
    ri.uri = uri;
    ri.mpCallback = p;

    std::string tmp = method;
    std::transform(tmp.begin(),tmp.end(),tmp.begin(),::toupper); 
    mCallbacks[tmp].push_back(ri);
}

void RESTEngine::removeCallBack(RESTCallBack* p)
{
    for (CallbackMap::iterator it = mCallbacks.begin(); it != mCallbacks.end(); it++)
    {
        for (auto it2=it->second.begin();it2!=it->second.end();it2++)
        {
            if (it2->mpCallback == p)
            {
                it->second.erase(it2);
                break;
            }
        }
    }
    delete p;
}

RESTEngine::ResponseCode RESTEngine::invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data)
{
    std::string tmp = method;
    std::transform(tmp.begin(),tmp.end(),tmp.begin(),::toupper); 
    if (mCallbacks.count(tmp) == 0) return MethodNotAllowed;
    auto& list = mCallbacks[tmp];

    std::string urlPart1 = url;
    std::string urlPart2 = "";
    size_t paramStart = url.find_first_of("?");
    if (paramStart!=std::string::npos)
    {
        urlPart1 = url.substr(0,paramStart);
        urlPart2 = url.substr(paramStart+1);
    }
    else
    {
        paramStart = url.find_first_of(" ");
        // avoiding a crash on MSVC2010
        if (paramStart==std::string::npos)
        {
            paramStart=url.size();
        }
        urlPart1 = url.substr(0,paramStart);
    }

    RESTCallBack *p = 0;
    std::smatch matches;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if (std::regex_match(urlPart1,matches, it->regex))
        {
            p = it->mpCallback;
            break;
        }
    }

    if (p)
    {
        p->call(j,urlPart2, data, matches);
        return OK;
    }
    return NotFound;
}

void RESTEngine::documentInterface(Dumais::JSON::JSON& json)
{
    json.addList("api");
    for (CallbackMap::iterator it = mCallbacks.begin(); it != mCallbacks.end(); it++)
    {
        std::string method = it->first;
        std::transform(method.begin(),method.end(),method.begin(),::toupper); 
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            Dumais::JSON::JSON& j = json["api"].addObject();
            it2->mpCallback->getDescription(j);
            j.addValue(it2->uri,"path");
            j.addValue(method,"method");
        }
    }

    return;
}

