/* 
Copyright (c) 2015 Patrick Dumais

http://www.dumaisnet.ca

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "utils/Logging.h"
#include "WebSocketServer.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace Dumais::WebSocket;

WebSocketServer::WebSocketServer(int port, int maxConnections)
{
    this->epollFD = -1;
    this->maxConnections = maxConnections;

    this->listenSocket = socket(AF_INET,SOCK_STREAM,0);
    int flags = fcntl(this->listenSocket,F_GETFL,0);
    fcntl(this->listenSocket, F_SETFL, flags | O_NONBLOCK);
    sockaddr_in sockadd;
    sockadd.sin_family=AF_INET;
    sockadd.sin_addr.s_addr=INADDR_ANY; //inet_addr(this->bindAddress.c_str());
    sockadd.sin_port=htons(port);
    int r = 1;
    setsockopt(this->listenSocket,SOL_SOCKET,SO_REUSEADDR,&r,sizeof(r));
    if (bind(this->listenSocket,(struct sockaddr *)&sockadd,sizeof(sockadd))<0)
    {
        LOG("Could not bind\r\n");
        close(this->listenSocket);
        //TODO: should make a method that checks this to detect errors
        this->listenSocket = -1;
        return;
    }
    listen(this->listenSocket,maxConnections);
    this->epollFD = epoll_create1(0);
    this->addFdToEpoll(this->listenSocket);
    this->epollEvents = new epoll_event[maxConnections+1];

    LOG("WebSocketServer started\r\n");
}

WebSocketServer::~WebSocketServer()
{
    this->abort();
}

bool WebSocketServer::work(int waitTimeout)
{
    if (this->listenSocket == -1) return false;

    int n = epoll_wait(this->epollFD, this->epollEvents, this->maxConnections + 2, waitTimeout);
    for (int i = 0; i < n; i++)
    {
        if (this->epollEvents[i].data.fd == this->listenSocket)
        {
            if (!this->processAccept())
            {
                LOG("Error while Accepting connection\r\n");
                this->abort();
            }
        }
        else
        {
            if (this->epollEvents[i].events & EPOLLIN)
            {
                this->processReceive(this->epollEvents[i].data.fd);
            }
        }
    }

    for (auto& it : this->webSockets)
    {
        if (!it.second->watchdog())
        {
            close(it.first);
            this->webSockets.erase(this->webSockets.find(it.first));
        }
    }
    
    // This will be called evrytime without even checking if the FD is available. 
    // We should probably check if the socket is ready to avoid failed send attempts
    this->processSend();
    return true;
}

void WebSocketServer::abort()
{
    LOG("WebSocketServer::abort\r\n");
    if (this->listenSocket >= 0)
    {
        close(this->listenSocket);
    }

    if (this->epollFD >= 0)
    {
        close(this->epollFD);
        delete[] this->epollEvents;
    }

    for (auto& it : webSockets)
    {
        if (it.second) delete it.second;
        close(it.first);
    }
    webSockets.clear();
    this->listenSocket = -1;
    this->epollFD = -1;
}

void WebSocketServer::processSend()
{
    char *buffer;
    size_t size, sent;

    for (auto& it : webSockets)
    {
        while ((size = it.second->getTxData(&buffer)) > 0)
        {
            sent = send(it.first,buffer, size, 0);
            if (sent == -1)
            {
                if (errno != EAGAIN)
                {
                    LOG("Got error "<<errno<<" while sending on socket\r\n");
                    this->abort();
                    break;
                }
                //if EAGAIN, we stop sending and we will get notified by EPOLL when we are ready again
            }
            else if (sent == 0)
            {
                //it->second->onDisconnected();
                webSockets.erase(webSockets.find(it.first));
                delete it.second;
                break;
            }
            else
            {
                // this will move us to the next buffer or further in current if partially sent.
                it.second->onDataSent(sent);
            }
        }
    }
}


void WebSocketServer::processReceive(int socket)
{
    auto it = this->webSockets.find(socket);
    if (it == this->webSockets.end()) return;

    char tmpBuffer[1024];  //TODO: should get a buffer from a buffer pool
    size_t n = 1;
    while (n > 0)
    {
        n = recv(socket, &tmpBuffer, 1024, 0);
        if (n == 0) // socket closed
        {
            this->webSockets.erase(it);
            if (it->second != 0)
            {
                delete it->second;
            }
            return;
        }
        else if (n == -1)
        {
            n = 0;
            if (errno != EAGAIN)
            {
                LOG("Error "<<errno<<" while recv()\r\n");
                this->abort();
            }
            break;
        }
        else
        {
            if (it->second)
            {
                if (!it->second->processData((char*)&tmpBuffer,n))
                {
                    close(socket);
                    delete it->second;
                    webSockets.erase(it);
                    return;
                }
            }
        }
    }
}


bool WebSocketServer::processAccept()
{
    int socket;

    // if the maximum number of clients has been reached, do not accept new ones. Leave them in the backlog
    if (this->webSockets.size() >= this->maxConnections) return true;
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

        this->webSockets[socket] = new WebSocket(this->mOnWebSocketRequest, 
                                                 this->mOnNewConnection, 
                                                 this->mOnConnectionClosed,
                                                 this->mOnMessage);
        addFdToEpoll(socket);
        LOG("Accepted new connection (total of "<< this->webSockets.size() << ")\r\n");
    }

    return true;
}

void WebSocketServer::addFdToEpoll(int fd)
{
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(this->epollFD, EPOLL_CTL_ADD, fd, &ev);
}

void WebSocketServer::setOnWebSocketRequest(std::function<bool(const std::string& request,
    std::map<std::string,std::string> protocols,
    std::string& chosenProtocol)> handler)
{
    this->mOnWebSocketRequest = handler;
}

void WebSocketServer::setOnNewConnection(std::function<void(WebSocket*)> handler)
{
    this->mOnNewConnection = handler;
}

void WebSocketServer::setOnConnectionClosed(std::function<void(WebSocket*)> handler)
{
    this->mOnConnectionClosed = handler;
}

void WebSocketServer::setOnMessage(std::function<void(WebSocket*,WebSocketMessage message)> handler)
{
    this->mOnMessage = handler;
}

