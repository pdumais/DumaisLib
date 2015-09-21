#pragma once
#include "TcpClient.h"
#include "ISocket.h"


namespace Dumais{
namespace WebServer{

class IClientFactory
{
public:
    virtual TcpClient* create(ISocket *sock) = 0;
};


template <class TFS, class TContext=ITcpClientContext>
class ClientFactory: public IClientFactory
{
public:
    ClientFactory(TcpClientObserver *obs)
    {
        this->observer = obs;
    }

    TcpClient* create(ISocket* sock)
    {
        TcpClient *c = new TcpClient(new TContext(), &framingStrategy);
        c->setObserver(observer);
        c->mSocket = sock;
        return c;
    }

private:
    TFS framingStrategy;
    TcpClientObserver *observer;
};

}
}
