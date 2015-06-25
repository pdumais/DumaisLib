#ifndef RESTENGINE_H
#define RESTENGINE_H

#include "RESTCallBack.h"
#include <regex>
#include <list>

struct ResourceIdentifier
{
    std::string uri;
    std::regex regex;
    RESTCallBack* mpCallback;
};

class RESTEngine{
private:
    std::list<ResourceIdentifier> mPOSTCallBackList;    
    std::list<ResourceIdentifier> mGETCallBackList;    
    std::list<ResourceIdentifier> mPUTCallBackList;    
    std::list<ResourceIdentifier> mDELETECallBackList;    
public:
    enum ResponseCode
    {
        OK,
        NotFound,
        MethodNotAllowed
    };

	RESTEngine();
	~RESTEngine();

    void addCallBack(std::string url, RESTMethod method, RESTCallBack* p);
    void removeCallBack(RESTCallBack* p);
    ResponseCode invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data);
    
    void documentInterface(Dumais::JSON::JSON& j);

};

#endif

