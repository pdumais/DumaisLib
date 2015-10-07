#include "TcpClient.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>


#define MAX_INTERNAL_TX_COUNT 4000
using namespace Dumais::WebServer;
using namespace Dumais::Utils;


TcpClient::TcpClient(ITcpClientContext* ctx, IFramingStrategy* fs)
{
    this->mFailed = false;
    this->observer = 0;
    this->framingStrategy = fs;
    this->context = ctx;
    this->currentSendBuffer.buffer = 0;
    this->currentMessage.complete = false;
    this->currentMessage.buffer = 0;
    this->currentMessage.size = 0;
    this->currentMessage.currentIndex = 0;
    this->txList = new MPSCRingBuffer<SendBuffer>(MAX_INTERNAL_TX_COUNT);
    this->mSocket = 0;
}

TcpClient::~TcpClient()
{
    SendBuffer s;
    while (this->txList->get(s))
    {
        delete[] s.buffer;
    }
    delete this->txList;
    if (this->context != 0)  delete this->context;
    if (this->mSocket) delete this->mSocket;
}

ISocket* TcpClient::getSocket()
{
    return this->mSocket;
}

void TcpClient::setObserver(TcpClientObserver* obs)
{
    this->observer = obs;
}


bool TcpClient::onReceive(char *data, size_t size)
{
    if (this->mFailed) return false;

    while (size > 0)
    { 
        int ret = this->framingStrategy->processData(data, size, this->currentMessage);
        if (ret < 0)
        {
            observer->onFramingError(this,ret);
            this->mFailed = true;
            return false;
        }

        // if more than one message was in the buffer, the strategy will return the index of the end
        // of the first message. So move on in the buffer and call the strategy again.
        size -= ret;
        data+=ret;
    
        if (this->currentMessage.complete)
        {
            observer->onClientMessage(this, this->currentMessage.buffer, this->currentMessage.size);
            this->currentMessage.complete = false;
            this->currentMessage.buffer = 0; // it was the engine's job to delete the buffer.
            this->currentMessage.size = 0;
            this->currentMessage.currentIndex = 0;
        }
    }

    return true;
}
void TcpClient::setControlEventFd(int fd)
{
    this->controlEventFd = fd;
}

void TcpClient::sendData(char* buffer, size_t size, bool copyInNewBuffer)
{
    SendBuffer sendBuffer;
    if (copyInNewBuffer)
    {
        sendBuffer.buffer = new char[size];
        sendBuffer.mustDeleteBuffer = true;
        memcpy(sendBuffer.buffer, buffer, size);
    }
    else
    {
        sendBuffer.buffer = buffer;
        sendBuffer.mustDeleteBuffer = false;
    }
    sendBuffer.size = size;
    sendBuffer.currentIndex = 0;
    if (!txList->put(sendBuffer))
    {
        //TODO: error
        if (copyInNewBuffer)
        {
            delete sendBuffer.buffer;
        }
    }
    char msg[8] = {1,0,0,0,0,0,0,0};
    write(this->controlEventFd,(char*)&msg[0], 8);
}

size_t TcpClient::getTxData(char** buf)
{
    SendBuffer sendBuffer;
    if (currentSendBuffer.buffer == 0 && txList->get(sendBuffer))
    {
        currentSendBuffer = sendBuffer;
    }
    if (currentSendBuffer.buffer == 0) return 0;

    *buf = (char*)&currentSendBuffer.buffer[currentSendBuffer.currentIndex];
    return currentSendBuffer.size - currentSendBuffer.currentIndex;

}

void TcpClient::onDataSent(size_t size)
{
    // This is called by the reactor to tell the client that data requested earlier is sent.
    // so the client must move on to the next data to be sent.
    if (currentSendBuffer.buffer == 0) return;
    
    currentSendBuffer.currentIndex += size;
    if (currentSendBuffer.currentIndex > currentSendBuffer.size)
    {
        //TODO: error! Wtf?
    }
    else
    if (currentSendBuffer.currentIndex == currentSendBuffer.size)
    {
        if (currentSendBuffer.mustDeleteBuffer) delete[] currentSendBuffer.buffer;
        currentSendBuffer.buffer = 0;
    }
}

void TcpClient::onConnected()
{
    if (this->observer != 0) this->observer->onClientConnected(this);
}

void TcpClient::onDisconnected()
{
    if (this->observer != 0) this->observer->onClientDisconnected(this);
}
