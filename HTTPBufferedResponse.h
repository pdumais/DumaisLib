#pragma once
#include <string>
#include "HTTPResponse.h"

namespace Dumais{
namespace WebServer{
class HTTPBufferedResponse: public HTTPResponse
{
private:
    friend class HTTPProtocol;
    std::string mContent;
public:
    HTTPBufferedResponse();
    ~HTTPBufferedResponse();
    virtual size_t getResponseChunk(char* buf, size_t startIndex, size_t max);

};


}
}
