#include "HTTPFramingStrategy.h"
#include <regex>

using namespace Dumais::WebServer;

int HTTPFramingStrategy::processData(char* buffer, size_t size, FullMessageInfo& info)
{
    if (info.buffer == 0)
    {
        info.buffer = new char[size];
    }
    else
    {
        char* tmp = new char[size+info.size];
        memcpy(tmp,info.buffer,info.size);
        delete info.buffer;
        info.buffer = tmp;
    }
    memcpy((char*)&info.buffer[info.size],buffer,size);
    // search for end of header
    std::string message = info.buffer;
    size_t eoh = message.find("\r\n\r\n");
    if (eoh == std::string::npos)
    {
        // Full header was not received. continue to receive data.
        info.size+=size;

        // If the buffer is bigger than 2k and we still didn't receive the end of header, then reject this connection
        if (info.size > 2048)
        {
            delete info.buffer;
            return -2;
        }
        return size;
    }

    std::regex r("transfer-encoding: *chuncked",std::regex_constants::icase);
    std::smatch matches;
    if (std::regex_search(message,matches,r))
    {
        return -3;
    }

    std::regex r2("Content-Length: *(.*)",std::regex_constants::icase);
    unsigned int contentLength = 0;
    if (std::regex_search(message,matches,r2))
    {
        contentLength = atoi(matches[1].str().c_str());
    }

    // check if full message was received
    if ((info.size+size) < (eoh+4+contentLength))
    {
        // Not finished receiving content. Carry on
        info.size+=size;
        return size;
    }

    info.complete = true;
    
    // check if we have other data at the end of the message
    unsigned int delta = (eoh+4+contentLength)-info.size;
    if (delta < size)
    {
        info.size+=delta;
    } 
    else
    {
        info.size+=size;
    }
    return delta;
}

