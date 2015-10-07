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

#include <queue>
#include <map>
#include <string>
#include "utils/MPSCRingBuffer.h"
#include <sys/epoll.h>
#include "HTTPProtocolParser.h"
#include "WSProtocolParser.h"
#include <functional>

namespace Dumais
{
    namespace WebSocket
    {
        struct WebSocketMessage
        {
            unsigned char* buffer;
            size_t size;
            unsigned char type;
            bool fin;
        };

        enum class Protocol
        {
            HTTP,
            WebSocket
        };

        enum class WSState
        {
            Connected,
            Closing,
            Closed
        };

        class WebSocket
        {
        private:
            Protocol protocol;
            std::function<bool(const std::string& request,
                std::map<std::string,std::string> protocols,
                std::string& chosenProtocol)> mOnWebSocketRequest;
            std::function<void(WebSocket*)> mOnNewConnection;
            std::function<void(WebSocket*)> mOnConnectionClosed;
            std::function<void(WebSocket*,WebSocketMessage message)> mOnMessage;

            time_t closingTimeStamp;
            WSState connectionState;
            HTTPProtocolParser httpParser;
            WSProtocolParser wsParser;

            struct SendBuffer
            {
                char* buffer;
                size_t size;
                size_t currentIndex;
                bool mustDeleteBuffer;
            };
            Dumais::Utils::MPSCRingBuffer<SendBuffer>* txList;
            SendBuffer currentSendBuffer;

            void abortWebSocket();
            void closeWebSocket();
            void checkHTTPBuffer(char* buffer, size_t size);
            void sendData(const char* buffer, size_t size, bool takeBufferOwnership=false);
            void processPing(char* buf, size_t size);
            void processWholeWSPacket(char* buffer, size_t headerSize, size_t payloadSize);

        public:
            WebSocket(std::function<bool(const std::string& request,
                                         std::map<std::string,std::string> protocols,
                                         std::string& chosenProtocol)>,
                      std::function<void(WebSocket*)> handler1, 
                      std::function<void(WebSocket*)> handler2, 
                      std::function<void(WebSocket*,WebSocketMessage message)> handler3);
            ~WebSocket();

            bool processData(char* data, size_t size);
            size_t getTxData(char** buf);
            void onDataSent(size_t size);
            bool watchdog();

            void sendText(const std::string& text);
            void sendBinary(char* buffer, size_t size);
        };
    }
}
