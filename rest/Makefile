all: lib
tls: lib


SOURCESCPP = RESTEngine.cpp RESTCallBack.cpp RESTParameters.cpp
OBJECTS=$(SOURCESCPP:.cpp=.o)
CFLAGS=-I ../ -g -std=c++11
LDFLAGS=
STATICMODULE=rest.a

clean:
	-rm *.o
	-rm *.a
	-rm a.out

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@


lib: $(OBJECTS)
	$(AR) rcs $(STATICMODULE) $(OBJECTS)

