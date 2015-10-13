all: lib
tls: lib
OBJECTS = JSON.o List.o Object.o Value.o Parser.o
SOURCES=$(OBJECTS:.o=.cpp)

clean:
	-rm *.o
	-rm a.out
	-rm *.a

.cpp.o:
	$(CXX) -g -std=c++11 -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs json.a $(OBJECTS) 

