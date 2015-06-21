#ifndef RESTENGINE_H
#define RESTENGINE_H

#include "RESTCallBack.h"

class RESTEngine{
private:
    std::unordered_map<std::string,RESTCallBack*> mPOSTCallBackList;    
    std::unordered_map<std::string,RESTCallBack*> mGETCallBackList;    
    std::unordered_map<std::string,RESTCallBack*> mPUTCallBackList;    
    std::unordered_map<std::string,RESTCallBack*> mDELETECallBackList;    
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

