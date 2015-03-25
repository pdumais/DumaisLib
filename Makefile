all: lib


SOURCESCPP = RESTEngine.cpp RESTCallBack.cpp RESTParameters.cpp
SOURCESTEST = main.cpp
OBJECTS=$(SOURCESCPP:.cpp=.o)
OBJECTSTEST=$(SOURCESTEST:.cpp=.o)
CFLAGS=-I json/ -g -std=c++0x
LDFLAGS=
STATICMODULES= RESTEngine.a

clean:
	-rm *.o
	-rm bin/*.a
	-rm a.out
	cd json && make clean

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

.PHONY: json
json:
	mkdir -p bin
	cd json && make
	cp json/json.a bin/

lib: json $(OBJECTS) main.cpp
	$(AR) rcs bin/RESTEngine.a $(OBJECTS)
	$(CXX) $(CFLAGS) main.cpp bin/*.a

