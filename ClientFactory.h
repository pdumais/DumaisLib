#pragma once
#include "TcpClient.h"

namespace Dumais{
namespace WebServer{

class IClientFactory
{
public:
    virtual TcpClient* create() = 0;
};


template <class TFS, class TContext=ITcpClientContext>
class ClientFactory: public IClientFactory
{
public:
    ClientFactory(TcpClientObserver *obs)
    {
        this->observer = obs;
    }

    TcpClient* create()
    {
        TcpClient *c = new TcpClient(new TContext(), &framingStrategy);
        c->setObserver(observer);
        return c;
    }

private:
    TFS framingStrategy;
    TcpClientObserver *observer;
};

}
}
