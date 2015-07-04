#include "TcpServer.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace Dumais::WebServer;

char controlTxMessage[] = {1,0,0,0,0,0,0,0};
char controlRxMessage[] = {0,0,0,0,0,0,0,0};

TcpServer::TcpServer(IClientFactory* factory, int port, std::string bindAddr, int max)
{
    this->clientFactory = factory;
    this->listeningPort = port;
    this->bindAddress = bindAddr;
    this->maxConnections = max;
}
    
TcpServer::~TcpServer()
{
}

bool TcpServer::start()
{
    // create listening socket
    this->listenSocket = socket(AF_INET,SOCK_STREAM,0);
    int flags = fcntl(this->listenSocket,F_GETFL,0);
    fcntl(this->listenSocket, F_SETFL, flags | O_NONBLOCK);
    sockaddr_in sockadd;
    sockadd.sin_family=AF_INET;
    sockadd.sin_addr.s_addr=inet_addr(this->bindAddress.c_str());
    sockadd.sin_port=htons(this->listeningPort);
    char r = 1;
    setsockopt(this->listenSocket,SOL_SOCKET,SO_REUSEADDR,&r,sizeof(r));
    if (bind(this->listenSocket,(struct sockaddr *)&sockadd,sizeof(sockadd))<0) return false;
    listen(this->listenSocket,this->maxConnections);

    // create control pipe
    this->controlEventFd = eventfd(0,EFD_NONBLOCK);
    if (this->controlEventFd == -1) return false;

    // start thre reactor thread
    this->stopping = false;
    this->reactorThread = new std::thread(&TcpServer::run, this);

    return true;
}

void TcpServer::stop()
{
    stopping = true;
    write(this->controlEventFd,(char*)&controlTxMessage[0], 8);    

    if (this->reactorThread != 0) this->reactorThread->join();
}

void TcpServer::run()
{
    int efd = epoll_create1(0);
    struct epoll_event *events;

    addFdToEpoll(efd, this->listenSocket);
    addFdToEpoll(efd, this->controlEventFd);
    events = new epoll_event[this->maxConnections + 2];

    int n;
    while (!stopping)
    {
        // No need for a timeout. If we have something to do, we will trigger it with the control pipe
        n = epoll_wait(efd, events, this->maxConnections + 2, -1);
        for (int i = 0; i < n; i++)
        {
            if (events[i].data.fd == this->listenSocket)
            {
                if (!this->processAccept(efd)) abort();
            }
            else if (events[i].data.fd == this->controlEventFd)
            {
                //nothing to do. It was just to wake the thread up.
                read(this->controlEventFd, (char*)&controlRxMessage, 8);
            }    
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    this->processReceive(events[i].data.fd);
                }
            }
        }

        // Process all outgoing
        this->processSend();
    }
    // Kill all clients
    for (auto it = this->clientList.begin(); it != clientList.end(); it++)
    {
        close(it->first);
        delete it->second;
    }

    // cleanup
    close(this->listenSocket);
    close(this->controlEventFd);
    close(efd);
    delete[] events;
}

void TcpServer::removeFdFromEpoll(int efd, int fd)
{
    //There is no need to remove a closed socket from epoll. It will be removed automatically
}

void TcpServer::addFdToEpoll(int efd, int fd)
{
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev); 
}

void TcpServer::abort()
{
    this->stopping = true;
}

bool TcpServer::processAccept(int efd)
{
    if (this->stopping) return false;

    int socket;
    while (true)
    {
        socket = accept(this->listenSocket,0,0);
        if (socket == -1)
        {
            if (errno == EAGAIN)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        int flags = fcntl(socket,F_GETFL,0);
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
        TcpClient* client = this->clientFactory->create();
        clientList[socket] = client;
        client->setControlEventFd(this->controlEventFd);
        client->onConnected();
        addFdToEpoll(efd,socket);
    }

    return true;
}
int TcpServer::processReceive(int socket)
{
    auto it = clientList.find(socket);
    if (it == clientList.end()) return 0;
    
    char tmpBuffer[1024];  //TODO: should get a buffer from a buffer pool
    size_t n = 1;
    while (n > 0)
    {
        if (this->stopping)
        {
            n = 0;
            break;
        }
        n = recv(socket, &tmpBuffer, 1024, 0);
        if (n == 0)
        {
            it->second->onDisconnected();
            clientList.erase(it);
            delete it->second;
        }
        else if (n == -1)
        {
            n = 0;
            if (errno == ECONNRESET)
            {
                it->second->onDisconnected();
                clientList.erase(it);
                delete it->second;
            }
            else if (errno != EAGAIN) abort();
            break;
        }
        else
        {
            !it->second->onReceive((char*)&tmpBuffer[0], n);            
        }
    }
    
    return n;
}
int TcpServer::processSend()
{
    char *buffer;
    size_t size, sent;
    if (this->stopping) return 0;


    //TODO: I find it dramatic to iterate over all the connections. Maybe 
    //      I could create one eventFd for each connection to notify that
    //      Data is ready to be sent.
    for (auto it = clientList.begin(); it != clientList.end(); it++)
    {
        while ((size = it->second->getTxData(&buffer)) > 0)
        {
            sent = send(it->first,buffer, size, 0);    
            if (sent == -1)
            {
                if (errno != EAGAIN) abort();
                //if EAGAIN, we stop sending and we will get notified by EPOLL when we are ready again
            }
            else if (sent == 0)
            {
                it->second->onDisconnected();
                clientList.erase(it);
                delete it->second;
                break;
            }
            else
            {
                // this will move us to the next buffer or further in current if partially sent.
                it->second->onDataSent(sent);
            }
        }

        // after we are finished sending the tx queue, check if the client is failed and close the connection
        if (it->second->mFailed)
        {
            close(it->first);
            it->second->onDisconnected();
            delete it->second;
            clientList.erase(it);
        }
    }


}


