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
#include <string>
#include "MPSCRingBuffer.h"

class IWebSocketHandler;

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

struct WSHeader
{
    unsigned char opcode:4;
    unsigned char reserved:3;
    unsigned char fin:1;
    unsigned char payloadSize:7;
    unsigned char mask:1;
} __attribute__((packed));

class WebSocket
{
private:
    Protocol protocol;
    char* data;
    size_t currentBufferSize;
    size_t dataIndex;
    IWebSocketHandler* handler;
    unsigned int maxData;
    unsigned int expectedLength;
    time_t closingTimeStamp;
    WSState connectionState;

    struct SendBuffer
    {
        char* buffer;
        size_t size;
        size_t currentIndex;
        bool mustDeleteBuffer;
    };
    MPSCRingBuffer<SendBuffer>* txList;
    SendBuffer currentSendBuffer;

    void resetMessageBuffer();
    void abort();
    void close();
    void reject(int code, const std::string& response);
    void checkHTTPBuffer();
    void sendData(const char* buffer, size_t size, bool takeBufferOwnership=false);
    void sendWSData(int type, const char* buffer, size_t size);
    void switchProtocol(const std::string& key);
    void parseWSMessage(char* buf, size_t size);
    std::string getHeader(std::string header);

public:
    WebSocket(IWebSocketHandler* handler);
    ~WebSocket();

    bool processData(char* data, size_t size);
    size_t getTxData(char** buf);
    void onDataSent(size_t size);
    bool watchdog();

    void sendText(const std::string& text);
    void sendBinary(char* buffer, size_t size);
};
