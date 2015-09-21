#pragma once

#include "TcpEngine.h"
#include <string>
#include <map>
#include "HTTPProtocol.h"

namespace Dumais{
namespace WebServer{
struct WebSession
{
    unsigned int mExpires;
    std::string mNonce;
};

class IWebServerListener
{
public:
   virtual HTTPResponse* processHTTPRequest(HTTPRequest* request)=0;
   virtual void onConnectionOpen()=0;
   virtual void onConnectionClosed()=0;
};





class WebServer: public TcpEngine
{

private:
    std::map<TcpClient*,TcpClient*> clients;
    IWebServerListener* mpListener;

    std::map<std::string,std::string> mPasswords;
    std::map<std::string,WebSession> mSessions;
    unsigned int mSessionExpires;
    bool mNeedsAuth;

    std::string generateNonce();
    void clearExpiredSessions();
    void sendResponse(HTTPResponse* response, TcpClient *client);
    std::string computemd5(const std::string& st);
    bool validateAuthentication(HTTPRequest* req, HTTPAuth auth);

protected:
    virtual void onFramingError(TcpClient* client, int error);
    virtual void onClientConnected(TcpClient* client);
    virtual void onClientDisconnected(TcpClient* client);
    virtual void onClientMessage(TcpClient* client, char* buffer, size_t size);
    
public:
    WebServer(unsigned int port, const std::string& bindAddr, unsigned int maxConnections);
    void setListener(IWebServerListener* pListener);
    void requireAuth(const char* authFileName,unsigned int sessionExpiry);

    bool startSecure(char* certificatePath, char* privateKeyPath);
    bool start();
    void stop();

    int getConnectionCount();
};

}
}
