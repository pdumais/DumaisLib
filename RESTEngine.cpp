#include "RESTEngine.h"
#include <sstream>
#include <stdio.h>

RESTEngine::RESTEngine(){
}

RESTEngine::~RESTEngine()
{
    for (std::unordered_map<std::string,IRESTCallBack*>::iterator it = mCallBackList.begin();it!=mCallBackList.end();it++)
    {
        delete it->second;
    }
}

void RESTEngine::addCallBack(std::string url, IRESTCallBack* p)
{
    mCallBackList[url] = p;
}

void RESTEngine::removeCallBack(IRESTCallBack* p)
{
    for (std::unordered_map<std::string,IRESTCallBack*>::iterator it = mCallBackList.begin();it!=mCallBackList.end();it++)
    {
        if (it->second == p)
        {
            mCallBackList.erase(it);
            return;
        }
    }
}

RESTEngine::ResponseCode RESTEngine::invoke(Dumais::JSON::JSON& j,std::string url)
{
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
        //printf("DEBUG [%s]\r\n",urlPart1.c_str());
    if (mCallBackList.find(urlPart1) != mCallBackList.end())
    {
        //printf("DEBUG 2\r\n");
        mCallBackList[urlPart1]->call(j,urlPart2);
        return OK;
    }
    else
    {
        return NotFound;
    }

}

void RESTEngine::documentInterface(Dumais::JSON::JSON& json)
{
    json.addList("api");
    for (std::unordered_map<std::string,IRESTCallBack*>::iterator it = mCallBackList.begin();it!=mCallBackList.end();it++)
    {
        Dumais::JSON::JSON& j = json["api"].addObject();
        it->second->getDescription(j);
        j.addValue(it->first,"path");
    }

    return;
}

