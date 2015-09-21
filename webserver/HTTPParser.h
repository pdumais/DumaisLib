#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <string>
#include <map>

namespace Dumais{
namespace WebServer{
class HTTPParser{
private:
    std::string mURL;
    std::string mRequest;
    std::map<std::string,std::string> mParamList;
    unsigned int headerSize;

public:
	HTTPParser(const char* url);
	~HTTPParser();

    unsigned int getHeaderSize();
    std::string getRequest();
    std::string getParam(std::string name);
    std::map<std::string,std::string> getParamList();
};

}
}

#endif

