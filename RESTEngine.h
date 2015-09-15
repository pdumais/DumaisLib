#ifndef RESTENGINE_H
#define RESTENGINE_H

#include "RESTCallBack.h"
#include <regex>
#include <list>
#include <map>

struct ResourceIdentifier
{
    std::string uri;
    std::regex regex;
    RESTCallBack* mpCallback;
};

typedef std::list<ResourceIdentifier> CallbackList;
typedef std::map<std::string, CallbackList> CallbackMap;


class RESTEngine{
private:
    CallbackMap mCallbacks;    

public:
    enum ResponseCode
    {
        OK,
        NotFound,
        MethodNotAllowed
    };

	RESTEngine();
	~RESTEngine();

    void addCallBack(std::string url, std::string method, RESTCallBack* p);
    void removeCallBack(RESTCallBack* p);
    ResponseCode invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data) const;
    
    void documentInterface(Dumais::JSON::JSON& j);

};

#endif

