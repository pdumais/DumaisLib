#include "HTTPProtocol.h"
#include "HTTPBufferedResponse.h"
#define SIZEOFARRAY( a ) (sizeof( a ) / sizeof( a[ 0 ] ))

using namespace Dumais::WebServer;

HTTPResponse* HTTPProtocol::buildBufferedResponse(HTTPResponseCode code, const std::string& content, const std::string& contentType, const std::string& authHeader)
{
    HTTPBufferedResponse *r = new HTTPBufferedResponse();

    r->mContent = content;
    r->mResponseCode = code;
    r->mContentType = contentType;
    r->mAuthHeader = authHeader;

    return r;
}

