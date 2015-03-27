#include "WebSocketServer.h"
#include "IWebSocketHandler.h"

class Test: public IWebSocketHandler
{
public:
    bool onWebSocketRequest(const std::string& request)
    {
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

    void onMessage(WebSocket* ws, WebSocketMessage message)
    {
        std::string st;
        st.assign((char*)message.buffer,message.size);
        printf("Message: [%s]\r\n",st.c_str());
        ws->sendText("Oh yeah! It works!");
    }

};

int main(int argc, char** argv)
{
    Test test;
    WebSocketServer ws(8056,20, new Dumais::Logging::ConsoleLogger());
    ws.setWebSocketHandler(&test);
    while (1)
    {
        if (!ws.work(20)) break;
    }
}
