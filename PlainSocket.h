#pragma once
#include "ISocket.h"

namespace Dumais{
namespace WebServer{

class PlainSocket: public ISocket
{
public:
    PlainSocket(int socket);
    
    virtual void close();
    virtual ISocket* accept();
    virtual int read(char* buffer, size_t size);
    virtual int send(char* buffer, size_t size);
};

}
}

