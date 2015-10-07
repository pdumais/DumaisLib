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
#pragma once

#include <unordered_map>
#include <list>
#include "utils/Logging.h"
#include <functional>
#include "WebSocket.h"

namespace Dumais
{
    namespace WebSocket
    {

        class WebSocketServer
        {
        private:
            void addFdToEpoll(int fd);
            void processSend();
            void processReceive(int fd);
            bool processAccept();
            void abort();

            // maps WebSockets using associated bsd socket
            std::unordered_map<int,WebSocket*> webSockets;

            int maxConnections;
            int listenSocket;
            int epollFD;
            struct epoll_event *epollEvents;

            std::function<bool(const std::string& request,
                std::map<std::string,std::string> protocols,
                std::string& chosenProtocol)> mOnWebSocketRequest;
            std::function<void(WebSocket*)> mOnNewConnection;
            std::function<void(WebSocket*)> mOnConnectionClosed;
            std::function<void(WebSocket*,WebSocketMessage message)> mOnMessage;

        public:
            WebSocketServer(int port, int maxConnections);    
            ~WebSocketServer();    

            // reactor function
            bool work(int waitTimeout);
            void setOnWebSocketRequest(std::function<bool(const std::string& request,
                std::map<std::string,std::string> protocols,
                std::string& chosenProtocol)> handler);
            void setOnNewConnection(std::function<void(WebSocket*)> handler);
            void setOnConnectionClosed(std::function<void(WebSocket*)> handler);
            void setOnMessage(std::function<void(WebSocket*,WebSocketMessage message)> handler);
        };
    }
}



