all: lib
tls: lib
EXCEPTIONS=./main.cpp
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS+=-std=c++11 -g -I ../
LDFLAGS+=-lpthread -lcurl
tls:CFLAGS+=-DUSING_OPENSSL
tls:LDFLAGS+=-lssl -lcrypto

clean:
	-rm *.o
	-rm *.a

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs webserver.a $(OBJECTS) 

