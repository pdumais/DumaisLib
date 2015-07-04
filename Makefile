all: tests
EXCEPTIONS=main.cpp
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)

clean:
	-rm *.o
	-rm *.a
	-rm test

.cpp.o:
	$(CXX) -g -std=c++11 -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs webserver.a $(OBJECTS) 

tests: lib main.o
	g++ main.o webserver.a -std=c++0x -o test -lpthread -lcurl
