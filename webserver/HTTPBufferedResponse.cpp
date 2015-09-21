#include "HTTPBufferedResponse.h"

using namespace Dumais::WebServer;

HTTPBufferedResponse::HTTPBufferedResponse(): HTTPResponse()
{
}

HTTPBufferedResponse::~HTTPBufferedResponse()
{
}

size_t HTTPBufferedResponse::getResponseChunk(char* buf, size_t startIndex, size_t max)
{
    if (this->buildHeader(mContent.size()))
    {
        mContent = mHeader + mContent;
    }

    if (startIndex>=mContent.size()) return 0;
    size_t copied  = mContent.copy(buf,max,startIndex);

    return copied;
}
