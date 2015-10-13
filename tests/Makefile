all: tests
tls: tests
EXCEPTIONS=
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJECTS=$(SOURCES:.cpp=.out)
CFLAGS+=-std=c++11 -g -I ../
LDFLAGS+=-lpthread -lcurl
tls:CFLAGS+=-DUSING_OPENSSL
tls:LDFLAGS+=-lssl -lcrypto

clean:
	-rm *.o
	-rm *.a
	-rm *.out

.cpp.out:
	$(CXX) $(CFLAGS) $<  ../sdk/lib/*.a -std=c++11 -o $@ $(LDFLAGS)

tests: clean $(OBJECTS)

