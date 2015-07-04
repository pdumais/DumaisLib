#pragma once
#include "ClientFactory.h"
#include <map>
#include <thread>

namespace Dumais{
namespace WebServer{
class TcpServer
{
public:
    TcpServer(IClientFactory* factory, int port, std::string bindAddr, int max);
    ~TcpServer();    

    bool start();
    void run();
    void stop();

private:
    IClientFactory* clientFactory;
    int listeningPort;
    int listenSocket;
    int controlEventFd;
    int maxConnections;
    volatile bool stopping;
    std::map<int,TcpClient*> clientList;
    std::thread *reactorThread;
    std::string bindAddress;
    

    bool processAccept(int efd);
    int processReceive(int socket);
    int processSend();
    int processControl(uint32_t events);
    void abort();
    void addFdToEpoll(int efd, int fd);
    void removeFdFromEpoll(int efd, int fd);
};



}
}
