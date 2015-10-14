SOURCES = WebSocketServer.cpp WebSocket.cpp HTTPProtocolParser.cpp WSProtocolParser.cpp
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS=-I ../ -g -std=c++11
.cpp.o:
	g++ $(CFLAGS) -c $< -o $@


all: lib
tls: lib

test: lib
	g++ -g $(CFLAGS) main.cpp websocket.a

lib: $(OBJECTS)
	$(AR) rcs websocket.a $(OBJECTS)


clean:
	-rm *.o
	-rm *.a
	-rm a.out
