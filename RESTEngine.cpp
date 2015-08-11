#include "RESTEngine.h"
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <regex>

RESTEngine::RESTEngine(){
}

RESTEngine::~RESTEngine()
{
    for (std::list<ResourceIdentifier>::iterator it = mPOSTCallBackList.begin(); it != mPOSTCallBackList.end(); ++it) delete it->mpCallback;
    for (std::list<ResourceIdentifier>::iterator it = mGETCallBackList.begin(); it != mGETCallBackList.end(); ++it) delete it->mpCallback;
    for (std::list<ResourceIdentifier>::iterator it = mPUTCallBackList.begin(); it != mPUTCallBackList.end(); ++it) delete it->mpCallback;
    for (std::list<ResourceIdentifier>::iterator it = mDELETECallBackList.begin(); it != mDELETECallBackList.end(); ++it) delete it->mpCallback;
}

void RESTEngine::addCallBack(std::string uri, RESTMethod method, RESTCallBack* p)
{
    ResourceIdentifier ri;
    ri.regex.assign(uri);
    ri.uri = uri;
    ri.mpCallback = p;

    switch (method)
    {
        case RESTMethod::POST: mPOSTCallBackList.push_back(ri); break;
        case RESTMethod::GET: mGETCallBackList.push_back(ri); break;
        case RESTMethod::PUT: mPUTCallBackList.push_back(ri); break;
        case RESTMethod::DELETE: mDELETECallBackList.push_back(ri); break;
    }
}

void RESTEngine::removeCallBack(RESTCallBack* p)
{
    for (auto it=mPOSTCallBackList.begin();it!=mPOSTCallBackList.end();it++) if (it->mpCallback == p) {mPOSTCallBackList.erase(it);break;}
    for (auto it=mGETCallBackList.begin();it!=mGETCallBackList.end();it++) if (it->mpCallback == p) {mGETCallBackList.erase(it);break;}
    for (auto it=mPUTCallBackList.begin();it!=mPUTCallBackList.end();it++) if (it->mpCallback == p) {mPUTCallBackList.erase(it);break;}
    for (auto it=mDELETECallBackList.begin();it!=mDELETECallBackList.end();it++) if (it->mpCallback == p) {mDELETECallBackList.erase(it);break;}
    delete p;
}

RESTEngine::ResponseCode RESTEngine::invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data)
{
    RESTMethod m;
    auto list = &mPOSTCallBackList;
    std::string tmp = method;
    std::transform(tmp.begin(),tmp.end(),tmp.begin(),::toupper); 
    if (tmp=="POST")
    {
        m = RESTMethod::POST;
        list = &mPOSTCallBackList;
    }
    else if (tmp=="GET")
    {
        m = RESTMethod::GET;
        list = &mGETCallBackList;
    }
    else if (tmp=="PUT")
    {
        m = RESTMethod::PUT;
        list = &mPUTCallBackList;
    }
    else if (tmp=="DELETE")
    {
        m = RESTMethod::DELETE;
        list = &mDELETECallBackList;
    }
    else
    {
        return MethodNotAllowed;
    }

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
            paramStart=url.size();
        urlPart1 = url.substr(0,paramStart);
    }

    RESTCallBack *p = 0;
    std::smatch matches;
    for (auto it = list->begin(); it != list->end(); ++it)
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
    for (auto it = mPOSTCallBackList.begin(); it != mPOSTCallBackList.end(); ++it)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it->mpCallback->getDescription(j);
        j.addValue(it->uri,"path");
        j.addValue("POST","method");
    }
    for (auto it = mGETCallBackList.begin(); it != mGETCallBackList.end(); ++it)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it->mpCallback->getDescription(j);
        j.addValue(it->uri,"path");
        j.addValue("GET","method");
    }
    for (auto it = mPUTCallBackList.begin(); it != mPUTCallBackList.end(); ++it)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it->mpCallback->getDescription(j);
        j.addValue(it->uri,"path");
        j.addValue("PUT","method");
    }
    for (auto it = mDELETECallBackList.begin(); it != mDELETECallBackList.end(); ++it)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it->mpCallback->getDescription(j);
        j.addValue(it->uri,"path");
        j.addValue("DELETE","method");
    }

    return;
}

