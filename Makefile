SOURCES = WebSocketServer.cpp WebSocket.cpp Logging.cpp sha1.cpp base64.cpp HTTPProtocolParser.cpp WSProtocolParser.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.cpp.o:
	g++ -g -std=c++11 -c $< -o $@


all: lib
	g++ -g -std=c++11 main.cpp websocket.a

lib: $(OBJECTS)
	$(AR) rcs websocket.a $(OBJECTS)


clean:
	-rm *.o
	-rm *.a
	-rm a.out
