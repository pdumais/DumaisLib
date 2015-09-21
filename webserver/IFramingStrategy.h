#pragma once
#include <string>

namespace Dumais{
namespace WebServer{
struct FullMessageInfo
{
    bool complete;
    size_t size;
    size_t currentIndex;
    char* buffer;
};

class IFramingStrategy
{
public:
    /*  process the chuck of data in buffer (of size 'size') and build the message in 'info'.
        the FullMessageInfo will be initialized with .buffer = 0. It is the responsibility of
        the strategy to allocate memory and to resize it as it receives more data over subsequent
        iterations. Once the strategy detects that a full message was read, it will set the 
        .complete flag in FullMessageInfo. after returning from that function, the buffer
        will be sent to the engine for further processing and it will be the engine's responsibilty
        to free the buffer.

        It is possible that more than one message are concatenated in the receive buffer.
        When the strategy detects the end of a message, it will return the position
        of the last byte of the parsed message relative to the receive buffer. The strategy
        will be called again with a buffer starting at that position (and size changed).
        FullMessageInfo.buffer will have been reset to 0.
        
        if a framing error occurs, returning -1 will abort the connection
    */
    virtual int processData(char* buffer, size_t size, FullMessageInfo& info) = 0;
};

}
}
