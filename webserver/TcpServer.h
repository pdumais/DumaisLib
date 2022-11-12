#pragma once
#include "ClientFactory.h"
#include <map>
#include <thread>
#include "ISocket.h"
#include <functional>

namespace Dumais{
namespace WebServer{
class TcpServer
{
public:
    TcpServer(IClientFactory* factory, int port, std::string bindAddr, int max);
    ~TcpServer();    

    bool start();
    void setStopEventHandler(std::function<void()> handler);
    void setAsyncQueueEventHandler(std::function<void()> handler);
    bool setSecurity(char* certificatePath, char* privateKeyPath);
    void run();
    void stop();
    void notifyAsyncQueue();

private:
    IClientFactory* clientFactory;
    int listeningPort;
    ISocket* listenSocket;
    int controlEventFd;
    int maxConnections;
    volatile bool stopping;
    std::map<int,TcpClient*> clientList;
    std::thread *reactorThread;
    std::string bindAddress;
    std::string certificatePath;    
    std::string privateKeyPath;    
    std::function<void()> stopEvent;
    std::function<void()> asyncQueueEvent;


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
