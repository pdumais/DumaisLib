#pragma once
#include <string>
#include <vector>
#include <map>

namespace Dumais{
namespace WebServer{

struct HTTPAuth
{
    std::string username;
    std::string nonce;
    std::string response;
    std::string uri;
};

// This is used to parse a request. Not to build one
class HTTPRequest
{
private:
    std::map<std::string,std::string> getAuthFields(std::string st);

    std::string mHeader;
    char* mBuffer;
    size_t mSize;

    std::string getHeader(const std::string& header);

public:
    HTTPRequest(char* buffer, size_t size);
    ~HTTPRequest();

    bool  accepts(const std::string& accept);
    std::string getURL();
    std::string getMethod();
    HTTPAuth getAuthorization(); 
    std::string getCookie(const std::string& name);
    bool wantsAuthenticationHack();
    std::string getData();

};

}
}
