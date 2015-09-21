#pragma once
#include <sys/types.h>

#ifdef USING_OPENSSL
#define SECURE_IMPLEMENTATION true
#else
#define SECURE_IMPLEMENTATION false
#endif

namespace Dumais{
namespace WebServer{

class ISocket
{
protected:
    int mSocket;

public:
    int getSocket() { return mSocket; }

    virtual void close() = 0;
    virtual ISocket* accept() = 0;
    virtual int read(char* buffer, size_t size) = 0;
    virtual int send(char* buffer, size_t size) = 0;
};

}
}

