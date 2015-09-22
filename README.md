# DumaisLib
This is a library containing several utilities for some of my projects. Previously, the libraries were all individual but it became more simple
to just merge all projects together.


Modules
==============

- Rest [README](rest/README.md)
- JSON [README](json/README.md)
- WebSocket [README](websocket/README.md)
- WebServer [README](webserver/README.md)

Compiling
==============
Invoking "make" in the root folder of the project will build all modules. The output will be stored in form
of an SDK in the "sdk" folder. sdk/include/projectX will contain all include files for projectX and static libraries
will be stored in sdk/lib/projectX.a

It is thus possible to link against only what is needed. For example, one might link against webserver.a and not 
with json.a because it is not needed. But be aware that some projects have inter-dependencies such as the "rest" project
that requires the "json" library.

The build process does not generate dynamic librairies. 
