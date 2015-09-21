#pragma once
#include "HTTPResponse.h"
#include "HTTPRequest.h"

namespace Dumais{
namespace WebServer{

class HTTPProtocol
{
public:
    enum HTTPParseStatus
    {
        Continue,
        Ready,
        Error,
        ErrorTooLarge
    };

    static HTTPResponse* buildBufferedResponse(HTTPResponseCode code, const std::string& content, const std::string& contentType, const std::string& authHeader="");
};

}
}
