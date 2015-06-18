#ifndef RESTENGINE_H
#define RESTENGINE_H

#include "RESTCallBack.h"

class RESTEngine{
private:
    std::unordered_map<std::string,IRESTCallBack*> mPOSTCallBackList;    
    std::unordered_map<std::string,IRESTCallBack*> mGETCallBackList;    
    std::unordered_map<std::string,IRESTCallBack*> mPUTCallBackList;    
    std::unordered_map<std::string,IRESTCallBack*> mDELETECallBackList;    
public:
    enum ResponseCode
    {
        OK,
        NotFound,
        MethodNotAllowed
    };

	RESTEngine();
	~RESTEngine();

    void addCallBack(std::string url, RESTMethod method, IRESTCallBack* p);
    void removeCallBack(IRESTCallBack* p);
    ResponseCode invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data);
    
    void documentInterface(Dumais::JSON::JSON& j);

};

#endif

