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
#include "IWebSocketHandler.h"
#include <functional>
#include <algorithm>
#include "sha1.h"
#include "base64.h"

#define MAX_DATA 1024
#define MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_CONTINUE 0x00
#define WS_TEXT 0x01
#define WS_BINARY 0x02
#define WS_CLOSE 0x08
#define WS_PING 0x09
#define WS_PONG 0x0A

#define CLOSING_TIMEOUT 5

using namespace Dumais::WebSocket;

WebSocket::WebSocket(IWebSocketHandler* h)
{
    this->maxData = MAX_DATA;
    this->handler = h;
    this->protocol = Protocol::HTTP;
    this->currentBufferSize = this->maxData+1;
    this->data = new char[this->currentBufferSize];
    this->dataIndex = 0;
    this->currentSendBuffer.buffer = 0;
    this->txList = new MPSCRingBuffer<SendBuffer>(1000);
}

WebSocket::~WebSocket()
{
    if (this->handler) this->handler->onConnectionClosed(this);
    LOG("WebSocket closed\r\n");
    delete[] this->data;
}

void WebSocket::reject(int code, const std::string& response)
{
    std::stringstream ss;
    ss << "HTTP/1.1 " << code << " " << response << "\r\nContent-type: text/html\r\nAccess-Control-Allow-Origin: *\r\nContent-length: 0\r\n\r\n";
    this->sendData(ss.str().c_str(), ss.str().size());
}

void WebSocket::checkHTTPBuffer()
{
    if (this->dataIndex <4) return;

    this->data[this->dataIndex] = 0;
    std::string str(this->data);
    if (str.compare(str.size() - 4, 4, "\r\n\r\n") != 0) return; //TODO: search 1st occurence instead of end. And read whole message if there is a body
    if (str.compare(0, 4, "GET ") != 0) return; //TODO: send unacceptable

    size_t space = str.find(" ",4);
    if (space == std::string::npos) return;
    
    std::string req = str.substr(4,space-4);

    if (this->handler)
    {
        if (this->handler->onWebSocketRequest(req))
        {
            this->switchProtocol(this->getHeader("Sec-WebSocket-Key"));
            return;
        }
    }
    this->reject(404,"Not Found");

}

void WebSocket::switchProtocol(const std::string& key)
{
    std::string accept = key+MAGIC;

    SHA1 sha;
    sha.addBytes(accept.c_str(),accept.size());
    accept = base64_encode(sha.getDigest(),20);

    std::stringstream ss;
    ss << "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " << accept << "\r\n\r\n";

    this->sendData(ss.str().c_str(), ss.str().size());
    this->protocol = Protocol::WebSocket;
    this->dataIndex = 0;
    this->expectedLength = sizeof(WSHeader);
    this->connectionState = WSState::Connected;
    if (this->handler) this->handler->onNewConnection(this);
}

bool WebSocket::processData(char* buf, size_t size)
{   
    if (this->protocol == Protocol::HTTP)
    {
        size_t tmp = this->dataIndex + size;
        if (tmp >= this->maxData)
        {   
            this->reject(431, "Request Header Fields Too Large");
            return false;
        }
        memcpy((char*)&this->data[this->dataIndex], buf, size);
        this->dataIndex = tmp;
        this->checkHTTPBuffer();
    }
    else
    {
        int toRead;
        while (size)
        {
            // if we got to much data, just read up until the expected size is reached. We will read the rest
            // on the next iteration.
            if ((this->dataIndex+size) > this->expectedLength) toRead = this->expectedLength-this->dataIndex; else toRead = size;
//LOG("Loop: %x %x %x %x\r\n",this, size, toRead,this->expectedLength);
            parseWSMessage(buf, toRead);
            size-=toRead;
            buf+=toRead;
        }
        if (this->connectionState == WSState::Closed) return false;
    }

    return true;
}

void WebSocket::parseWSMessage(char* buf, size_t size)
{
    // in this function, we are guaranteed that the size is not bigger than 1 message
    // but it can be smaller.
    memcpy((char*)&this->data[this->dataIndex], buf, size);
    this->dataIndex += size;
    // if we have a full message, process it. Otherwise continue accumulating the data
    if (this->dataIndex == this->expectedLength)
    {
        // At this point, we are guaranteed to have the minimum size for the header.
        // since this->dataIndex == this->expectedLength and xpectedLength cannot be smaller
        // than minimum header size
        WSHeader* header = (WSHeader*)&this->data[0];
        size_t headerSize = sizeof(WSHeader);

        if (header->mask)
        {
            headerSize += 4;
        }
        else
        {
            // must disconnect because this is not allowed as per RFC
            // "All frames sent from client to server have this bit set to 1." - RFC6455, section 5.2
            this->close();
            LOG("Client sent unmasked data.\r\n");
            return;
        }

        if (header->payloadSize == 126)
        {
            headerSize += 2;
        } 
        else if (header->payloadSize == 127)
        {
            headerSize += 8;
        }

        // if we were expecting the minimum size, update it to the real required size
        if (this->expectedLength == sizeof(WSHeader))
        {
            this->expectedLength = headerSize;
        }
        
        if (this->dataIndex == headerSize)
        {
            // We have the full header, now calculate the full payload size
            size_t payloadSize = header->payloadSize;
            if (payloadSize == 126) // 16bit value
            {
                payloadSize = ((size_t)this->data[2]<<8)|((size_t)this->data[3]);
            }
            else if (payloadSize == 127) // 64bit value
            {
                if (sizeof(size_t) != 8)
                {
                    LOG("64bit payload size not supported here\r\n");
                    this->close();
                    return;
                }
                //TODO: in assembly, this could be done in 2 instructions
                payloadSize = ((size_t)this->data[2]<<56)|
                              ((size_t)this->data[3]<<48)|
                              ((size_t)this->data[4]<<40)|
                              ((size_t)this->data[5]<<32)|
                              ((size_t)this->data[6]<<24)|
                              ((size_t)this->data[7]<<16)|
                              ((size_t)this->data[8]<<8)|
                              ((size_t)this->data[9]);
            }
            this->expectedLength = headerSize + payloadSize;
            
            // Reajust buffer size if bigger than what was previously allocated
            if (this->expectedLength > this->currentBufferSize)
            {
                char *newBuffer = new char[this->expectedLength];
                memcpy(newBuffer,this->data,this->dataIndex);
                delete this->data;
                this->data = newBuffer;
                this->currentBufferSize = this->expectedLength;
            }
        }
        else if (this->expectedLength > headerSize)
        {
            LOG("Message for "<<this<<", opcode = " << (int)header->opcode << "\r\n");
            // we were expecting the full thing and we have it.
            unsigned char* mask = (unsigned char*)&this->data[headerSize-4];
            unsigned char* payload = (unsigned char*)&this->data[headerSize];
            size_t payloadSize = this->expectedLength - headerSize;

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
                if (this->handler) this->handler->onMessage(this, msg);
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
                    this->close();
                }
                this->abort();
            }
            else if (header->opcode == WS_PING)
            {
                this->processPing(this->data, payloadSize+headerSize);
            }
            else
            {
                LOG("Received invalid opcode\r\n");
                this->close();
                return;
            }

            // Message complete. reset values for next one.
            this->resetMessageBuffer();
        }
    }
}

void WebSocket::resetMessageBuffer()
{
    this->dataIndex = 0;
    this->expectedLength = sizeof(WSHeader);
    if (this->currentBufferSize > MAX_DATA)
    {
        // To avoid using to much memory
        this->currentBufferSize = MAX_DATA;
        delete this->data;
        this->data = new char[this->currentBufferSize];
    }
}

std::string WebSocket::getHeader(std::string header)
{
    this->data[this->dataIndex] = 0;
    std::string str(this->data);
    std::string h = str;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(header.begin(), header.end(), header.begin(), ::tolower);
    size_t n = h.find("\r\n"+header,0);
    if (n == std::string::npos) return "";
    n+=(2+header.size()+1);
    size_t end = str.find("\r\n",n);
    size_t start = str.find_first_not_of(" ",n);
    if (end == std::string::npos) return "";
    if (start == std::string::npos) return "";

    return str.substr(start,end-start);
}

size_t WebSocket::getTxData(char** buf)
{
    if (this->connectionState != WSState::Connected) return 0;
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
        this->abort();    
    }
}

void WebSocket::abort()
{
    // We just abort the connection immediately
    this->connectionState = WSState::Closed;
}

void WebSocket::close()
{
    char* packet = new char[sizeof(WSHeader)];
    WSHeader* header = (WSHeader*)packet;
    header->reserved = 0;
    header->mask = 0;
    header->fin = 1;
    header->opcode = WS_CLOSE;
    header->payloadSize = 0;
    this->sendData(packet, sizeof(WSHeader), true);

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
            this->abort();
            return false;
        }
    }
    return true;
}

void WebSocket::sendWSData(int type, const char* buffer, size_t size)
{
    char* packet = new char[size+sizeof(WSHeader)+8]; // make it bigger in case we need 8 bytes for payloadSize
    size_t headerSize = sizeof(WSHeader);
    WSHeader* header = (WSHeader*)packet;
    header->reserved = 0;
    header->mask = 0;
    header->fin = 1; // we do not support fragmentation yet
    header->opcode = type;
    if (size > 125)
    {
        if (size > 65535)
        {
            //headerSize += 8;
            //header->payloadSize = 127; //rfc6455 section 5.2
            LOG("64 bit size is not supporte yet for TX\r\n");
            return;
        }
        else
        {
            headerSize += 2;
            header->payloadSize = 126; //rfc6455 section 5.2
            packet[2] = (char)(size>>8);
            packet[3] = (char)(size&0xFF);
        }
    }
    else
    {
        header->payloadSize = size;
    }
    memcpy((char*)&packet[headerSize],buffer,size);
    this->sendData(packet, size+headerSize, true);
}

void WebSocket::sendText(const std::string& text)
{
    this->sendWSData(WS_TEXT,text.c_str(),text.size());
}

void WebSocket::sendBinary(char* buffer, size_t size)
{
    this->sendWSData(WS_BINARY, buffer, size);
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

