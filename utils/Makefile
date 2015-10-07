tls:lib
EXCEPTIONS=./main.cpp
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -lcurl

clean:
	-rm *.o
	-rm *.a

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs utils.a $(OBJECTS) 

all: lib 
	#g++ main.o webserver.a -std=c++0x -o test $(LDFLAGS)
