#include "WebSocketServer.h"

using namespace Dumais::WebSocket;

int main(int argc, char** argv)
{
    WebSocketServer ws(8056,20, new Dumais::WebSocket::ConsoleLogger());

    // I'm using lambdas for callbacks here, but you prefer to put your code in a 
    // function then you would do: ws.setOnWebSocketRequest(functionName); or for a method:
    // ws.setOnWebSocketRequeststd::bind(ClassX::MethodY,&instanceOfClassX,std::placeholders::_1));
    ws.setOnWebSocketRequest([](const std::string& request,
        std::map<std::string,std::string> protocols,
        std::string& chosenProtocol)
    {
        if (protocols.find("protocol1") == protocols.end()) return false;
        chosenProtocol = "protocol1";
        if (request == "/ws") return true;
        return false;
    });
    ws.setOnNewConnection([](WebSocket* ws){printf("New websocket connection %x\r\n", ws);});
    ws.setOnConnectionClosed([](WebSocket* ws){printf("websocket connection closed %x\r\n", ws);});
    ws.setOnMessage([](WebSocket* ws, WebSocketMessage message){
        std::string st;
        st.assign((char*)message.buffer,message.size);
        printf("Message: [%s]\r\n",st.c_str());
        ws->sendText("Oh yeah! It works!");
    });

    while (1)
    {
        if (!ws.work(20)) break;
    }
}
