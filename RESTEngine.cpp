#include "RESTEngine.h"
#include <sstream>
#include <stdio.h>
#include <algorithm>

RESTEngine::RESTEngine(){
}

RESTEngine::~RESTEngine()
{
    for (auto& it : mPOSTCallBackList) delete it.second;
    for (auto& it : mGETCallBackList) delete it.second;
    for (auto& it : mPUTCallBackList) delete it.second;
    for (auto& it : mDELETECallBackList) delete it.second;
}

void RESTEngine::addCallBack(std::string url, RESTMethod method, IRESTCallBack* p)
{
    switch (method)
    {
        case RESTMethod::POST: mPOSTCallBackList[url] = p; break;
        case RESTMethod::GET: mGETCallBackList[url] = p; break;
        case RESTMethod::PUT: mPUTCallBackList[url] = p; break;
        case RESTMethod::DELETE: mDELETECallBackList[url] = p; break;
    }
}

void RESTEngine::removeCallBack(IRESTCallBack* p)
{
    for (auto it=mPOSTCallBackList.begin();it!=mPOSTCallBackList.end();it++) if (it->second == p) {mPOSTCallBackList.erase(it); return;}
    for (auto it=mGETCallBackList.begin();it!=mGETCallBackList.end();it++) if (it->second == p) {mGETCallBackList.erase(it); return;}
    for (auto it=mPUTCallBackList.begin();it!=mPUTCallBackList.end();it++) if (it->second == p) {mPUTCallBackList.erase(it); return;}
    for (auto it=mDELETECallBackList.begin();it!=mDELETECallBackList.end();it++) if (it->second == p) {mDELETECallBackList.erase(it); return;}
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
        urlPart1 = url.substr(0,paramStart);
    } 


    if (list->find(urlPart1) != list->end())
    {
        (*list)[urlPart1]->call(j,urlPart2, data);
        return OK;
    }
    return NotFound;
}

void RESTEngine::documentInterface(Dumais::JSON::JSON& json)
{
    json.addList("api");
    for (auto& it : mPOSTCallBackList)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it.second->getDescription(j);
        j.addValue(it.first,"path");
        j.addValue("POST","method");
    }
    for (auto& it : mGETCallBackList)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it.second->getDescription(j);
        j.addValue(it.first,"path");
        j.addValue("GET","method");
    }
    for (auto& it : mPUTCallBackList)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it.second->getDescription(j);
        j.addValue(it.first,"path");
        j.addValue("PUT","method");
    }
    for (auto& it : mDELETECallBackList)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it.second->getDescription(j);
        j.addValue(it.first,"path");
        j.addValue("DELETE","method");
    }

    return;
}

