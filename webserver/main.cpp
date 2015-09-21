#include <stdio.h>

#include "WebServer.h"
#include <unistd.h>
#include <curl/curl.h>
#include <string>
#include <string.h>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include "stdlib.h"
#include <sys/time.h>
#include <errno.h>

using namespace Dumais::WebServer;

std::string lastURL;

class Test: public IWebServerListener
{
public:
    virtual HTTPResponse* processHTTPRequest(HTTPRequest* request)
    {
        lastURL = request->getURL();
        printf("%s [%s]\r\n",request->getMethod().c_str(),lastURL.c_str());
        return HTTPProtocol::buildBufferedResponse(HTTPResponseCode::OK,request->getURL(),"");
    }

    virtual void onConnectionOpen()
    {
        printf("Connection open\r\n");
    }

    virtual void onConnectionClosed()
    {
        printf("Connection closed\r\n");
    }

};

char *curlbuf = 0;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t s = size*nmemb;
    if (curlbuf) delete curlbuf;
    curlbuf = new char[s+1];
    memcpy(curlbuf,ptr,s);
    curlbuf[s] = 0;
    return s;
}

long query(std::string url, bool auth=false)
{
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    if (auth)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
        curl_easy_setopt(curl, CURLOPT_USERPWD, "pat:pat");
    }
    CURLcode code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);    

    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    printf("http code: %i  %i\r\n",http_code, code);
    return http_code;
}

long post(std::string url)
{
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,"test=1&test2=2");
    curl_easy_setopt(curl, CURLOPT_COOKIE, "meow=cat; woof=dog;");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    CURLcode code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

    return http_code;
}


int getURL(std::string server, int port)
{
    std::string ret="";
    bool chunked = false;

    sockaddr_in sockadd;
    int sock=socket(AF_INET,SOCK_STREAM,0);
    memset(&sockadd,0,sizeof(sockadd));
    struct hostent *he;
    he = gethostbyname(server.c_str());
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(port);
    memcpy(&sockadd.sin_addr, he->h_addr_list[0], he->h_length);
    bool content = false;
    std::string st;
    if (connect(sock,(struct sockaddr *)&sockadd, sizeof(sockadd)) >= 0)
    {
        return sock;
    }
    return 0;
}



#define ASSERT(f) if (f) printf("%i: OK\r\n",__LINE__); else printf("%i: ------- > FAIL\r\n",__LINE__);


int main(int argc, char** argv)
{
    Test test;
    char rcv[1024];
    bool started;
    std::string ret;
    
#ifdef USING_OPENSSL
    WebServer webs(5556,"0.0.0.0",10);
    webs.setListener(&test);
    started = webs.startSecure("cert.crt","key.key");
    ASSERT(started);
    if (!started) return -1;

    // test ssl
    ASSERT(query("https://127.0.0.1:5556/test/123")==200);
    if (curlbuf !=0) ret = curlbuf;
    ASSERT(ret == "/test/123");
    webs.stop();
#endif

    WebServer web(5555,"0.0.0.0",100);
    web.setListener(&test);
    started = web.start();
    ASSERT(started);
    if (!started) return -1;


    // test post
    ASSERT(post("http://127.0.0.1:5555/test")==200);
    ret = curlbuf;
    ASSERT(ret == "/test");

    // test get and echoing url no auth
    ASSERT(query("http://127.0.0.1:5555/test/123")==200);
    ret = curlbuf;
    ASSERT(ret == "/test/123");

    // test sending in 10bytes packets increments (fragmented)
    {
    std::stringstream ss;
    ss << "GET /test242 HTTP/1.1\r\n";
    ss << "Host: 127.0.0.1\r\n";
    ss << "Connection: close\r\n";
    ss << "Content-length: 3\r\n";
    ss << "\r\nabc";
    const char* buf = ss.str().c_str();
    int i = 0;
    int sock = getURL("127.0.0.1",5555);
    while (i < ss.str().size())
    {
        int s = ss.str().size()-i;
        if (s > 10) s = 10;
        send(sock,(char*)&buf[i],s,0);
        i+=10;
    }
    recv(sock,(char*)&rcv[0],1,0); // just receive 1 byte, we will ignore the response
    close(sock);
    ASSERT(lastURL=="/test242");
    }

    // test 2 messages in one packet
    {
    std::stringstream ss;
    ss << "GET /test243 HTTP/1.1\r\n";
    ss << "Host: 127.0.0.1\r\n";
    ss << "Connection: close\r\n";
    ss << "Content-length: 3\r\n";
    ss << "\r\nabc";
    ss << "GET /test244 HTTP/1.1\r\n";
    ss << "Host: 127.0.0.1\r\n";
    ss << "Connection: close\r\n";
    ss << "Content-length: 3\r\n";
    ss << "\r\nabc";
    const char* buf = ss.str().c_str();
    int sock = getURL("127.0.0.1",5555);
    send(sock,ss.str().c_str(),ss.str().size(),0);
    recv(sock,(char*)&rcv[0],1,0); // just receive 1 byte, we will ignore the response
    close(sock);
    ASSERT(lastURL=="/test244");
    }

    // test that chunked transfers are refused
    {
    std::stringstream ss;
    ss << "GET /test243 HTTP/1.1\r\n";
    ss << "Host: 127.0.0.1\r\n";
    ss << "Connection: close\r\n";
    ss << "Transfer-Encoding: chunked\r\n";
    ss << "\r\nabc";
    const char* buf = ss.str().c_str();
    int sock = getURL("127.0.0.1",5555);
    send(sock,ss.str().c_str(),ss.str().size(),0);
    recv(sock,(char*)&rcv[0],12,0); // just receive 1 byte, we will ignore the response
    rcv[12]=0;
    ASSERT(!strcmp((char*)&rcv[9],"406"));
    }

    // test 100 connections at the same time
    {
    std::stringstream ss;
    ss << "GET /test/100 HTTP/1.1\r\n";
    ss << "Host: 127.0.0.1\r\n";
    ss << "Connection: close\r\n";
    ss << "Content-length: 3\r\n";
    ss << "\r\nabc";
    int sockets[100];
    for (int i = 0; i < 100; i++) sockets[i] = getURL("127.0.0.1",5555);
    for (int i = 0; i < 100; i++) send(sockets[i],ss.str().c_str(),ss.str().size(),0);
    for (int i = 0; i < 100; i++)
    {
        recv(sockets[i],(char*)&rcv[0],12,0); // just receive 1 byte, we will ignore the response
        rcv[12]=0;
        ASSERT(!strcmp((char*)&rcv[9],"200"));
    }
    }

    // test sending invalid data
    {
    std::stringstream ss;
    for (int i=0;i<200;i++) ss << "GET asdfsdfsfdfs  HTTP/1.1 dsfkjnasdfnsmdnfsmndfmn.,sdfasdf";
    const char* buf = ss.str().c_str();

    int sock = getURL("127.0.0.1",5555);
    send(sock,ss.str().c_str(),ss.str().size(),0);
    recv(sock,(char*)&rcv[0],12,0); // just receive 1 byte, we will ignore the response
    close(sock);
    rcv[12]=0;
    ASSERT(!strcmp((char*)&rcv[9],"413"));
    }

    // test with auth, bad password
    web.requireAuth("passwd",30);
    ASSERT(query("http://127.0.0.1:5555/test/123")==401);

    // test with auth, good password
    ASSERT(query("http://127.0.0.1:5555/test/123",true)==200);
    ret = curlbuf;
    ASSERT(ret == "/test/123");

    web.stop();
}
