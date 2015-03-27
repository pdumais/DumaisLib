SOURCES = main.cpp WebSocketServer.cpp WebSocket.cpp Logging.cpp sha1.cpp base64.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.cpp.o:
	g++ -g -std=c++11 -c $< -o $@

all: $(OBJECTS)
	g++ -g $(OBJECTS)

clean:
	rm *.o
