#ifndef RESTENGINE_H
#define RESTENGINE_H

#include "json/JSON.h"
#include <regex>
#include <list>
#include <map>
#include <memory>

class RESTCallBack; // Convenient forward declaration avoiding to include RESTCallback.h

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
        Created,
        Accepted,
        BadRequest,
        Forbidden,
        NotFound,
        MethodNotAllowed,
        NotImplemented,
        ServiceUnavailable
    };

    RESTEngine();
    ~RESTEngine();

    void addCallBack(std::string url, std::string method, RESTCallBack* p);
    void removeCallBack(RESTCallBack* p);
    ResponseCode invoke(Dumais::JSON::JSON& j, std::string url, const std::string& method, const std::string& data,
                        std::shared_ptr<void> userData = NULL) const;
    
    void documentInterface(Dumais::JSON::JSON& j);

    /**
     * @brief documentSwaggerInterface
     * @param json that will be filled with the swagger schema
     * @param version of the api
     * @param title of the api
     * @param description of the api
     * @param schemes: http, https, ...
     * @param host: localhost, myserver, ...
     * @param basePath: /api, /v1, /api/v1, ...
     */
    void documentSwaggerInterface(Dumais::JSON::JSON& json,
                               const std::string &version, const std::string &title, const std::string &description,
                               const std::string &schemes, const std::string &host, const std::string &basePath
                               );
};

#endif

