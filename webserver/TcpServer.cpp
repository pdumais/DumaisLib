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
#include "PlainSocket.h"
#include "SecureSocket.h"
#include "utils/Logging.h"

using namespace Dumais::WebServer;

char controlTxMessage[] = {1,0,0,0,0,0,0,0};
char controlRxMessage[] = {0,0,0,0,0,0,0,0};

TcpServer::TcpServer(IClientFactory* factory, int port, std::string bindAddr, int max)
{
    this->clientFactory = factory;
    this->listeningPort = port;
    this->bindAddress = bindAddr;
    this->maxConnections = max;
    this->listenSocket = 0;
}
    
TcpServer::~TcpServer()
{
    if (this->listenSocket != 0) delete this->listenSocket;
}

void TcpServer::setStopEventHandler(std::function<void()> handler)
{
    stopEvent = handler;
}

void TcpServer::setAsyncQueueEventHandler(std::function<void()> handler)
{
    asyncQueueEvent = handler;
}

bool TcpServer::setSecurity(char* certificatePath, char* privateKeyPath)
{
    if (!SECURE_IMPLEMENTATION) return false;
    this->privateKeyPath = privateKeyPath;
    this->certificatePath = certificatePath;
    return true;
}

bool TcpServer::start()
{
    // create listening socket
    int s = socket(AF_INET,SOCK_STREAM,0);
    int flags = fcntl(s,F_GETFL,0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
    sockaddr_in sockadd;
    sockadd.sin_family=AF_INET;
    sockadd.sin_addr.s_addr=inet_addr(this->bindAddress.c_str());
    sockadd.sin_port=htons(this->listeningPort);
    int r = 1;
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&r,sizeof(r));
    if (bind(s,(struct sockaddr *)&sockadd,sizeof(sockadd))<0) return false;
    listen(s,this->maxConnections);

    // create control pipe
    this->controlEventFd = eventfd(0,EFD_NONBLOCK);
    if (this->controlEventFd == -1) return false;

    if (this->privateKeyPath=="")
    {
        this->listenSocket = new PlainSocket(s);
    }
    else
    {
        SecureSocket *ss = new SecureSocket(s);
        ss->initServer(this->certificatePath, this->privateKeyPath);
        this->listenSocket = ss;
    }

    // start thre reactor thread
    this->stopping = false;
    this->reactorThread = new std::thread(&TcpServer::run, this);

    return true;
}

void TcpServer::notifyAsyncQueue()
{
    write(this->controlEventFd,(char*)&controlTxMessage[0], 8);    
}

void TcpServer::stop()
{
    LOG("setting stopping to true because of stop()");
    stopping = true;
    write(this->controlEventFd,(char*)&controlTxMessage[0], 8);    

    if (this->reactorThread != 0) this->reactorThread->join();
}

void TcpServer::run()
{
    int efd = epoll_create1(0);
    struct epoll_event *events;

    addFdToEpoll(efd, this->listenSocket->getSocket());
    addFdToEpoll(efd, this->controlEventFd);
    events = new epoll_event[this->maxConnections + 2];

    int n;
    while (!stopping)
    {
        // No need for a timeout. If we have something to do, we will trigger it with the control pipe
        n = epoll_wait(efd, events, this->maxConnections + 2, -1);
        for (int i = 0; i < n; i++)
        {
            if (events[i].data.fd == this->listenSocket->getSocket())
            {
                if (!this->processAccept(efd))
                {
                    abort();
                }
            }
            else if (events[i].data.fd == this->controlEventFd)
            {
                //TODO: Should reall all of it
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
        if (asyncQueueEvent) asyncQueueEvent();
        this->processSend();
    }

    LOG("TCP server thread exit");

    // Kill all clients
    for (auto it = this->clientList.begin(); it != clientList.end(); it++)
    {
        close(it->first);
        delete it->second;
    }
    // cleanup
    this->listenSocket->close();
    close(this->controlEventFd);
    close(efd);
    delete[] events;

    if (stopEvent) stopEvent();
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
    LOG("setting stopping to true because of abort");
    this->stopping = true;
}

bool TcpServer::processAccept(int efd)
{
    if (this->stopping) return false;

    int socket;
    while (true)
    {
        ISocket *sock = this->listenSocket->accept();
        if (sock == 0)
        {
            if (errno == EAGAIN)
            {
                return true;
            }
            else
            {
                LOG("error while accepting. returned " << errno);
                return false;
            }
        }
        socket = sock->getSocket();
        int flags = fcntl(socket,F_GETFL,0);
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
        TcpClient* client = this->clientFactory->create(sock);
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
    
    TcpClient* client = it->second;

    char tmpBuffer[1024];  //TODO: should get a buffer from a buffer pool
    size_t n = 1;
    while (n > 0)
    {
        if (this->stopping)
        {
            n = 0;
            break;
        }
        n = client->getSocket()->read((char*)&tmpBuffer[0], 1024);

        if (n == 0)
        {
            client->onDisconnected();
            clientList.erase(it);
            delete client;
        }
        else if (n == -1)
        {
            n = 0;
            if (errno == ECONNRESET)
            {
                client->onDisconnected();
                clientList.erase(it);
                delete client;
            }
            else 
            {
                if (errno != EAGAIN)
                {
                    LOG("Read error. Returned " << errno);
                    abort();
                }
            }
            break;
        }
        else
        {
            client->onReceive((char*)&tmpBuffer[0], n);            
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
        TcpClient* client = it->second;
        while ((size = client->getTxData(&buffer)) > 0)
        {
            sent = client->getSocket()->send(buffer, size);    
            if (sent == -1)
            {
                if (errno != EAGAIN)
                {
                    LOG("write error. returned " << errno);
                    abort();
                }
                //if EAGAIN, we stop sending and we will get notified by EPOLL when we are ready again
            }
            else if (sent == 0)
            {
                client->onDisconnected();
                clientList.erase(it);
                delete it->second;
                break;
            }
            else
            {
                // this will move us to the next buffer or further in current if partially sent.
                client->onDataSent(sent);
            }
        }

        // after we are finished sending the tx queue, check if the client is failed and close the connection
        if (client->mFailed)
        {
            client->getSocket()->close();
            client->onDisconnected();
            delete client;
            clientList.erase(it);
        }
    }


}


