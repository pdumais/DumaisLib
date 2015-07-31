# webserver
This is a simple implementation of a HTTP server. It is definitely not up to specs.
I use this in many of my personal projects and it proved good so far.

Implementation
==============
The server is single threaded and uses epoll internally. The server will spawn a worker thread that
will handle all the job. Sending data to clients can be done from another thread since the internal
message queue is thread safe.

Compiling
==============
Makefile will generate a webserver.a static library to you can link your projects with.

Usage
==============
All you need to do is implement the IWebServerListener interface
```
class HTTPTest: public IWebServerListener
{
public:
    virtual HTTPResponse* processHTTPRequest(HTTPRequest* request)
    {
        cout << request->getURL();
        cout << request->getMethod();
        return HTTPProtocol::buildBufferedResponse(HTTPResponseCode::OK,"Some data","text/plain");
    }

    virtual void onConnectionOpen()
    {
    }

    virtual void onConnectionClosed()
    {
    }

};

```

And then start an instance of the server and listen to it. The server will spawn its own thread.

```
HTTPTest test;

// bind on all interfaces on port 80 and allow 100 connections simultaneously
WebServer web(80,"0.0.0.0",100);
web.setListener(&test);
web.start();
...

web.stop();

```

if you require auth (digest auth supported only), then create a file and put
some username:password pairs (separated by colons) then reference the file like this:
```
web.requireAuth("/path/to/passwdfile",30 /*Session expiry delay*/);  
```

That would force the client to you digest authentication. Then a session will be created
and the session ID will be sent in a cookie in the response. To avoid authenticating again, 
the client should pass the session ID in the cookies on every request. The session will stay
valid for the period of time set as the 3rd parameter of requireAuth().


Security
==============
You can build with TLS support with "make tls". This build will require the OpenSSL library.
You executable, that links with this static lib, will also need to link with libcrypto and libssl.
To use TLS, simply replace
```
web.start();
```
with
```
web.startSecure("/path/to/PublicKey","/path/to/PrivateKey");
```

If you attempt to call startSecure with a version of the library that wasn't compiled with TLS support, the call
will return false.


