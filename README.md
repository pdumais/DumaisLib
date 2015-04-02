# websocket
A c++ websocket server library

THREADING
==============
The websocket libary requires you to to provide your own loop and call the WebSocketServer::work() method.
Each call the work() gives the server a timeslice and allows it to make a call to epoll_wait. That means that
you can create a full application using only 1 thread with one main loop where you give a time slice to
all your submodules.

Any received messages will be processed in from the work() call and event handlers will be called from there.
So an events will be called on the same thread than the work() call.

When a message needs to be sent out, it will be queued in a thread-safe queue and sent out during the current
work() call. If you wish to send data out from another thread, it is perfectly safe to do so since the
send queue is thread-safe.

FRAGMENTATION
==============
Fragmentation handling is left to consummer. The library will notify the consummer when a message is received
and will indicate if it is a Text,Binary or Continuation message and if the FIN flag is set

Fragmentation is not supported when sending messages out from the server.

SECURITY
==============
SSL is not supported at the moment. Although it would be pretty easy to snap openSSL in WebSocketServer.cpp.
There are not plans on doing this for the moment.

PING/PONG
==============
The library supports ping/pong. But it was never tested because my browser doesn't send pings.
RFC6455, sectioon 5.5.3 says: A Pong frame sent in response to a Ping frame must have identical 
"Application data" as found in the message body of the Ping frame being replied to. So I'm not
sure if it means I should unmask it or not. This implementation does not unmask it, the message
is just echoed back with the opcode changed.

RFC6455
==============
The library is not 100% RFC6455 compliant. Some things left to do (among others) are:
    - support fragmentation on TX from server
    - support 64bit payload size on RX and TX
    - support Close reasons
    - support subprotocols    
    - some other stuff...

Usage
==============
```
#include "WebSocketServer.h"
#include "IWebSocketHandler.h"

//Create a callback handler
class Test: public IWebSocketHandler
{
public:
    bool onWebSocketRequest(const std::string& request,
        std::map<std::string,std::string> protocols,
        std::string& chosenProtocol)
    {
        if (protocols.find("protocol1") == protocols.end()) return false;
        chosenProtocol = "protocol1";
        if (request == "/ws") return true;
        return false;
    }

    void onNewConnection(WebSocket* ws)
    {
        printf("New websocket connection %x\r\n", ws);
    }

    void onConnectionClosed(WebSocket* ws)
    {
        printf("websocket connection closed %x\r\n", ws);
    }

    void onMessage(WebSocket* ws, unsigned char* buf, size_t size)
    {
        std::string st;
        st.assign((char*)buf,size);
        printf("Message: [%s]\r\n",st.c_str());
        ws->sendText("Oh yeah! It works!");
    }

};

int main(int argc, char** argv)
{

    // listen on port 8056, max 20 connections, use default console Logger.
    WebSocketServer ws(8056,20, new Dumais::Logging::ConsoleLogger());
    
    // Set the callback hander
    Test test; // this is the callback handler
    ws.setWebSocketHandler(&test);

    while (1)
    {
        // give it some processing time. Max 20ms waiting time for epoll_wait
        if (!ws.work(20)) break;
    }
}
```
