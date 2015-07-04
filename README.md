# webserver
This is a simple implementation of a HTTP server. It is definitely not up to specs.
I use this in many of my personal projects and it proved good so far.

Implementation
==============
The server is single threaded and uses epoll internally. The server will spawn a worker thread that
will handle all the job. Sending data to clients can be done from another thread since the internal
message queue is thread safe.

Usage
==============
All you need to do is implement the IWebServerListener interface
```
class HTTPTest: public IWebServerListener
{
public:
    virtual HTTPResponse* processHTTPRequest(HTTPRequest* request)
    {
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
