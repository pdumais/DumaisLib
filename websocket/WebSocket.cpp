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
#include "Logging.h"
#include "WebSocket.h"
#include <string.h>
#include <sstream>
#include <functional>
#include <algorithm>


#define CLOSING_TIMEOUT 5

using namespace Dumais::WebSocket;

WebSocket::WebSocket(std::function<bool(const std::string& request,
                                         std::map<std::string,std::string> protocols,
                                         std::string& chosenProtocol)> handler,
                     std::function<void(WebSocket*)> handler1, 
                     std::function<void(WebSocket*)> handler2, 
                     std::function<void(WebSocket*,WebSocketMessage message)> handler3)
{
    this->mOnWebSocketRequest = handler;
    this->mOnNewConnection = handler1;
    this->mOnConnectionClosed = handler2;
    this->mOnMessage = handler3;
    this->protocol = Protocol::HTTP;
    this->currentSendBuffer.buffer = 0;
    this->txList = new MPSCRingBuffer<SendBuffer>(1000);
}

WebSocket::~WebSocket()
{
    if (this->mOnConnectionClosed) this->mOnConnectionClosed(this);
    LOG("WebSocket closed\r\n");
}

void WebSocket::checkHTTPBuffer(char* buffer, size_t size)
{
    HTTPRequest req = this->httpParser.parseIncoming(buffer, size);    
    LOG("state="<<req.status<<" method=["<<req.type<<"] res=["<<req.resource<<"]\r\n");
    if (req.status == HTTPRequest::Incomplete) return;

    if (req.status == HTTPRequest::Error) return; //TODO: should disconnect
    if (req.type != "GET") return; //TODO: send unacceptable and disconnect

    if (this->mOnWebSocketRequest)
    {
        std::string chosen;
        // notify the handler what request we received. If the handler is ready to serve this resource,
        // it will return true and we will then switch protocol.
        if (this->mOnWebSocketRequest(req.resource, req.protocols, chosen))
        {
            if (req.headers.find("sec-websocket-key") == req.headers.end()) return; //TODO send unacceptable and disconnect
            std::string str = this->httpParser.switchProtocol(req.headers["sec-websocket-key"],chosen);
            this->sendData(str.c_str(), str.size());
            this->protocol = Protocol::WebSocket;
            this->connectionState = WSState::Connected;
            if (this->mOnNewConnection) this->mOnNewConnection(this);
            return;
        }
    }
    std::string error = this->httpParser.reject(404,"Not Found");
    this->sendData(error.c_str(), error.size());
}

bool WebSocket::processData(char* buf, size_t size)
{   
    if (this->protocol == Protocol::HTTP)
    {
        this->checkHTTPBuffer(buf,size);
    }
    else
    {
        // We could have more than one messages in the same packet, so loop until we read them all.
        while (size)
        {
            WSParsingResult result = this->wsParser.parseWSMessage(buf, size);
            size-=result.dataRead;
            buf+=result.dataRead;

            if (result.status == WSParsingResult::Error)
            {
                this->closeWebSocket();
                break;
            } 
            else if (result.status == WSParsingResult::MessageReady)
            {
                this->processWholeWSPacket(result.buffer, result.headerSize, result.payloadSize);
            }
        }
        if (this->connectionState == WSState::Closed) return false;
    }

    return true;
}

void WebSocket::processWholeWSPacket(char* buffer, size_t headerSize, size_t payloadSize)
{
    WSHeader* header = (WSHeader*)&buffer[0];
    LOG("Message for "<<this<<", opcode = " << (int)header->opcode << "\r\n");
    // we were expecting the full thing and we have it.
    unsigned char* mask = (unsigned char*)&buffer[headerSize-4];
    unsigned char* payload = (unsigned char*)&buffer[headerSize];

    if (header->opcode == WS_CONTINUE || header->opcode == WS_TEXT || header->opcode == WS_BINARY)
    {
        // decode using mask. RFC6455, section 5.3
        for (int i = 0; i < payloadSize; i++)
        {
            payload[i] ^= mask[i%4];
        }
        WebSocketMessage msg;
        msg.buffer = payload;
        msg.size = payloadSize;
        msg.type = header->opcode;
        msg.fin = header->fin;
        if (this->mOnMessage) this->mOnMessage(this, msg); 
    }
    else if (header->opcode == WS_CLOSE)
    {
        // if the Close was initiated by the client then we are in Connected state
        // we must send a Close confirmation. But if we are in Closing state, it means that
        // this message is a confirmation from the Close that we sent earlier. In that
        // case, don't send a confirmation and just close the socket.
        // RFC6455 section 5.5.1
        if (this->connectionState == WSState::Connected)
        {
           //TODO: should reply with the same payload. This is non-compliant
           this->closeWebSocket(); 
        }
        this->abortWebSocket(); 
    }
    else if (header->opcode == WS_PING)
    {
        this->processPing(buffer, payloadSize+headerSize); 
    }
    else
    {
        LOG("Received invalid opcode\r\n");
        this->closeWebSocket(); 
        return;
    }

    // Message complete. reset values for next one.
    this->wsParser.resetMessageBuffer();
}

size_t WebSocket::getTxData(char** buf)
{
    if (this->protocol == Protocol::WebSocket && this->connectionState != WSState::Connected) return 0;
    SendBuffer sendBuffer;
    if (currentSendBuffer.buffer == 0 && txList->get(sendBuffer))
    {
        currentSendBuffer = sendBuffer;
    }
    if (currentSendBuffer.buffer == 0) return 0;

    *buf = (char*)&currentSendBuffer.buffer[currentSendBuffer.currentIndex];
    return currentSendBuffer.size - currentSendBuffer.currentIndex;
}

void WebSocket::onDataSent(size_t size)
{
    if (currentSendBuffer.buffer == 0) return;

    currentSendBuffer.currentIndex += size;
    if (currentSendBuffer.currentIndex > currentSendBuffer.size)
    {
        //TODO: error!how is this even possible?
    }
    else if (currentSendBuffer.currentIndex == currentSendBuffer.size)
    {
        if (currentSendBuffer.mustDeleteBuffer) delete[] currentSendBuffer.buffer;
        currentSendBuffer.buffer = 0;
    }
}

void WebSocket::sendData(const char* buffer, size_t size, bool takeBufferOwnership)
{
    SendBuffer sendBuffer;

    if (takeBufferOwnership)
    {
        // we take buffer ownership. Caller will not delete it. It is our job.
        sendBuffer.buffer = const_cast<char*>(buffer);
    }
    else
    {
        // Caller keeps buffer ownership, so we must create a new one
        sendBuffer.buffer = new char[size];
        memcpy(sendBuffer.buffer, buffer, size);
    }
    sendBuffer.mustDeleteBuffer = true;
    sendBuffer.size = size;
    sendBuffer.currentIndex = 0;
    if (!txList->put(sendBuffer))
    {
        LOG("Tx queue overflow");
        delete[] sendBuffer.buffer;
        this->abortWebSocket();    
    }
}

void WebSocket::abortWebSocket()
{
    // We just abort the connection immediately
    this->connectionState = WSState::Closed;
}

void WebSocket::closeWebSocket()
{
    auto ret = this->wsParser.generateCloseMessage();
    this->sendData(ret.second, ret.first, true);

    this->connectionState = WSState::Closing;
    time(&this->closingTimeStamp);
}
    
bool WebSocket::watchdog()
{
    if (this->protocol != Protocol::WebSocket) return true;

    if (this->connectionState == WSState::Closing)
    {
        time_t t;
        time(&t);

        if (t >= (this->closingTimeStamp+CLOSING_TIMEOUT))
        {
            this->abortWebSocket();
            return false;
        }
    }
    return true;
}

void WebSocket::sendText(const std::string& text)
{
    auto ret = this->wsParser.sendData(WS_TEXT,text.c_str(),text.size());
    if (ret.second==0) return;
    this->sendData(ret.second, ret.first, true);
}

void WebSocket::sendBinary(char* buffer, size_t size)
{
    auto ret = this->wsParser.sendData(WS_BINARY, buffer, size);
    if (ret.second==0) return;
    this->sendData(ret.second, ret.first, true);
}

void WebSocket::processPing(char* buf, size_t size)
{
    // It is unclear if we need to decode the payload when responding to a ping request
    // because RFC6455, sectioon 5.5.3 says A Pong frame sent in response to a Ping frame 
    // must have identical "Application data" as found in the message body of the Ping frame being replied to.
    //
    // Therefore, I will not unmask the data
    WSHeader* header = (WSHeader*)buf;
    header->opcode = WS_PONG;

    // we should probably add this in front of the queue to give it more priority
    // but we would open ourselves to DOS attacks.
    this->sendData(buf, size, false);
    LOG("Ping. Pong!\r\n");
}

