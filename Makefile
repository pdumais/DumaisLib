all: tests
OBJECTS = JSON.o List.o Object.o Value.o Parser.o
SOURCES=$(OBJECTS:.o=.cpp)

clean:
	-rm *.o

.cpp.o:
	$(CXX) -g -std=c++11 -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs json.a $(OBJECTS) 

tests: lib
	g++ main.cpp json.a -std=c++0x
