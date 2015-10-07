#pragma once

#include <cstddef>
#include <queue>
#include "ITcpClientContext.h"
#include "IFramingStrategy.h"
#include "utils/MPSCRingBuffer.h"
#include "ISocket.h"

namespace Dumais{
namespace WebServer{
class TcpClientObserver;

// This object will receive raw data from the socket without having to deal
// with the socket. It will use a strategy to frame the data and pass it down the engine.
// You should not derive this class. You should use the TcpClientContext object to 
// attach extra information on the client.
class TcpClient
{
public:
    ~TcpClient();
    void sendData(char* buffer, size_t size, bool copyInNewBuffer);
    ISocket* getSocket();

private:
    friend class TcpServer;
    template <class T, class T2>
    friend class ClientFactory;

    ISocket *mSocket;

    TcpClient(ITcpClientContext* context, IFramingStrategy *framingStrategy);
    void setObserver(TcpClientObserver* obs);
    bool onReceive(char *data, size_t size);
    void onDataSent(size_t size);
    void onConnected();
    void onDisconnected();
    // the reactor will request data to be sent by the client. The client will pass its own buffer to
    // avoid copying data
    size_t getTxData(char** buf);
    void setControlEventFd(int fd);

    struct SendBuffer
    {
        char* buffer;
        size_t size;
        size_t currentIndex;
        bool mustDeleteBuffer;
    };

    Dumais::Utils::MPSCRingBuffer<SendBuffer>* txList;
    SendBuffer currentSendBuffer;
    ITcpClientContext* context;
    IFramingStrategy *framingStrategy; 
    TcpClientObserver* observer;
    FullMessageInfo currentMessage;
    int controlEventFd;
    bool mFailed;
};

class TcpClientObserver
{
public:
    //Note: Clients should only be accessed from the server thread since they
    //      can be deleted any time. external components must use: TcpEngine::sendMessage()

    virtual void onFramingError(TcpClient* client, int error) = 0;
    virtual void onClientConnected(TcpClient* client) = 0;
    virtual void onClientDisconnected(TcpClient* client) = 0;

    //WARNING: It is the consumer's responsibility to free the buffer provided by that function
    virtual void onClientMessage(TcpClient* client, char* buffer, size_t size) = 0;
};



}
}
