#ifndef RESTENGINE_H
#define RESTENGINE_H

#include "RESTCallBack.h"

class RESTEngine{
private:
    std::unordered_map<std::string,IRESTCallBack*> mCallBackList;    
public:
    enum ResponseCode
    {
        OK,
        NotFound
    };

	RESTEngine();
	~RESTEngine();

    void addCallBack(std::string url, IRESTCallBack* p);
    void removeCallBack(IRESTCallBack* p);
    ResponseCode invoke(Dumais::JSON::JSON& j,std::string url);
    
    void documentInterface(Dumais::JSON::JSON& j);

};

#endif

