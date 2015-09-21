#include "TcpEngine.h"
using namespace Dumais::WebServer;


TcpEngine::TcpEngine(IClientFactory *factory)
{
    this->clientFactory = factory;
    this->tcpServer = 0;
}

TcpEngine::~TcpEngine()
{
    if (this->tcpServer != 0) delete this->tcpServer;
    if (this->clientFactory != 0) delete this->clientFactory;
}

void TcpEngine::init(int port, std::string bindAddr, int maxConnections)
{
    this->tcpServer = new TcpServer(this->clientFactory,port, bindAddr, maxConnections);
}

bool TcpEngine::startSecure(char* certificatePath, char* privateKeyPath)
{
    if (this->tcpServer == 0) return false;
    if (!this->tcpServer->setSecurity(certificatePath, privateKeyPath)) return false;
    return this->start();
}

bool TcpEngine::start()
{
    if (this->tcpServer == 0) return false;
    return this->tcpServer->start();
}

void TcpEngine::stop()
{
    if (this->tcpServer == 0) return;
    this->tcpServer->stop();
}

