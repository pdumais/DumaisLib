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

SECURITY
==============
SSL is not supported at the moment. Although it would be pretty easy to snap openSSL in WebSocketServer.cpp.
There are not plans on doing this for the moment.

RFC6455
==============
The library is not 100% RFC6455 compliant. Some things left to do (among others) are:
    - support fragmentation on RX and TX
    - support 64bit payload size on RX and TX

