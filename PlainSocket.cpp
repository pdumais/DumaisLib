#include "PlainSocket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

using namespace Dumais::WebServer;

PlainSocket::PlainSocket(int socket)
{
    this->mSocket = socket;
}


void PlainSocket::close()
{
    ::close(this->mSocket);
}

ISocket* PlainSocket::accept()
{
    int s = ::accept(this->mSocket,0,0);
    if (s != -1)
    {
        return new PlainSocket(s);
    }
    return 0;
}

int PlainSocket::read(char* buffer, size_t size)
{
    return ::read(this->mSocket, buffer, size);
}

int PlainSocket::send(char* buffer, size_t size)
{
    return ::send(this->mSocket, buffer, size, 0);
}

