#include "WebServer.h"
#include "HTTPFramingStrategy.h"
#include <string.h>
#include <sstream>
#include <time.h>
#include <fstream>
#include "utils/md5.h"

using namespace Dumais::WebServer;


#define BUFSIZE 1024

/**
Notes on authorization:
    When auth is enabled and a request comes in, the server will reply with a 401 with a nonce.
    The client will resend the request with a response. If the response is accepted, then
    the server will create a session that will last 30 sec(or whatever is configured). That session ID
    will be sent in the 200 OK as a cookie. The server will validate session cookies every time and
    will not issue a 401 for other requests if the session is still valid.

*/
WebServer::WebServer(unsigned int port, const std::string& bindAddr, unsigned int maxConnections)
    :TcpEngine(new ClientFactory<HTTPFramingStrategy>(this))
{
    this->init(port, bindAddr, maxConnections);
    mNeedsAuth = false;
    mSessionExpires = 0;
    srand(time(0));
    mpListener = 0;
}

void WebServer::onFramingError(TcpClient* client, int error)
{
    HTTPResponse *resp= 0 ;

    if (error == -3)
    {
        resp = HTTPProtocol::buildBufferedResponse(NotImplemented,"","");
    }
    else if (error == -2)
    {
        resp = HTTPProtocol::buildBufferedResponse(RequestEntityTooLarge,"","");
    }

    if (resp == 0)
    {
        resp = HTTPProtocol::buildBufferedResponse(ServiceUnavailable,"","");
    }
    this->sendResponse(resp,client);
}

void WebServer::onClientConnected(TcpClient* client)
{
    clients[client] = client;
    if (mpListener!=0) mpListener->onConnectionOpen();

}

void WebServer::onClientDisconnected(TcpClient* client)
{
    if (mpListener!=0) mpListener->onConnectionClosed();
    clients.erase(client);
}

void WebServer::onClientMessage(TcpClient* client, char* buffer, size_t size)
{
    clearExpiredSessions();

    HTTPResponse* resp = 0;
    HTTPRequest* req = new HTTPRequest(buffer,size);
    HTTPAuth auth = req->getAuthorization();
    bool authOK = validateAuthentication(req,auth);
    if (authOK)
    {
        if (mpListener!=0)
        {
            resp = mpListener->processHTTPRequest(req);
            if (resp!=0 && auth.nonce!="" && mSessions.find(auth.nonce)!=mSessions.end())
            {
                std::string cookie = "session="+auth.nonce;
                resp->setCookie(cookie);
            }
            sendResponse(resp,client);
        }
        else
        {
            resp = HTTPProtocol::buildBufferedResponse(ServiceUnavailable,"","");
            sendResponse(resp,client);
        }
    }
    else
    {
        std::string nonce = "";
        WebSession ws;
        if (auth.nonce!="" && mSessions.find(auth.nonce) != mSessions.end())
        {
            nonce = auth.nonce;
            ws = mSessions[nonce];
        }
        else
        {
            nonce = generateNonce();
            ws.mNonce = nonce;
        }

        time_t t;
        time(&t);
        t+=mSessionExpires;
        ws.mExpires = t;
        mSessions[nonce] = ws;
        std::string content = "{\"status\" : \"Authorization needed\"}";
        std::string authHeader = "Digest realm=\"dumais\",nonce=\""+nonce+"\"";
        HTTPResponseCode code = Unauthorized;
        resp = HTTPProtocol::buildBufferedResponse(code,content,"application/json", authHeader);
        if (req->wantsAuthenticationHack()) resp->useAuthenticationHack();
        sendResponse(resp,client);
    }

    delete buffer;
}

void WebServer::setListener(IWebServerListener* pListener)
{
   mpListener = pListener;
}

bool WebServer::start()
{
    return TcpEngine::start();
}

bool WebServer::startSecure(char* cPath, char* pPath)
{
    return TcpEngine::startSecure(cPath, pPath);
}

void WebServer::stop()
{
    //TODO: should disconnect all clients
    TcpEngine::stop();
}

int WebServer::getConnectionCount()
{
    return clients.size();
}

std::string WebServer::generateNonce()
{
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string nonce;
    for (int i=0;i<20;i++)
    {
        nonce += alphanum[rand()%62];
    }
    return nonce;
}

bool WebServer::validateAuthentication(HTTPRequest* req, HTTPAuth auth)
{
    if (!mNeedsAuth) return true;

    std::string sessionCookie = req->getCookie("session");

    if (mSessions.find(sessionCookie) != mSessions.end()) return true;
    if (auth.nonce == "") return false;
    if (mSessions.find(auth.nonce) == mSessions.end()) return false;
    if (mPasswords.find(auth.username) == mPasswords.end()) return false;

    std::string a1 = auth.username+":dumais:"+mPasswords[auth.username];
    std::string ha1 = computemd5(a1);
    std::string a2 = std::string("GET:")+auth.uri;
    std::string ha2 = computemd5(a2);
    std::string a3 = ha1+":"+auth.nonce+":"+ha2;
    std::string ha3 = computemd5(a3);

    return (ha3 == auth.response);
}

std::string WebServer::computemd5(const std::string& st)
{
    Dumais::Utils::MD5 md5(st.c_str(),st.size());
    return md5.toHex();
}

void WebServer::clearExpiredSessions()
{
    time_t t;
    time(&t);
    for (auto it=mSessions.begin(); it!=mSessions.end(); it++)
    {
        if (it->second.mExpires < t) mSessions.erase(it);
    }
}


void WebServer::requireAuth(const char* authFileName, unsigned int sessionExpiry)
{
    mNeedsAuth = true;
    mSessionExpires = sessionExpiry;

    std::ifstream file(authFileName);
    std::string line;
    while (std::getline(file,line))
    {
        size_t delimiter = line.find(":");
        if (delimiter != std::string::npos)
        {
            std::string username = line.substr(0,delimiter);
            std::string password = line.substr(delimiter+1);
            mPasswords[username]=password;
        }

    }
}

void WebServer::sendResponse(HTTPResponse* response, TcpClient *client)
{
    if (response == 0) return;
    char buf[BUFSIZE];
    size_t index = 0;
    size_t r = 1;
    while (r>0)
    {
        r = response->getResponseChunk(buf, index, BUFSIZE);
        client->sendData(buf,r,true);
        index+=r;
    }
    delete response;
}

