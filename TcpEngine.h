#pragma once
#include "ClientFactory.h"
#include "ITcpClientContext.h"
#include "TcpServer.h"

namespace Dumais{
namespace WebServer{

class TcpEngine: public TcpClientObserver
{
public:
    TcpEngine(IClientFactory *factory);
    ~TcpEngine();

    void init(int port, std::string bindAddr, int maxConnections);
    bool startSecure(char* certificatePath, char* privateKeyPath);
    bool start();
    void stop();

private:
    TcpServer* tcpServer;
    IClientFactory *clientFactory;
};

}
}
