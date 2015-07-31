all: tests
tls: tests
EXCEPTIONS=./main.cpp
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -lcurl
tls:CFLAGS+=-DUSING_OPENSSL
tls:LDFLAGS+=-lssl -lcrypto

clean:
	-rm *.o
	-rm *.a
	-rm test

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs webserver.a $(OBJECTS) 

tests: lib main.o
	g++ main.o webserver.a -std=c++0x -o test $(LDFLAGS)
