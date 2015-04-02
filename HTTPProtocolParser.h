/*
Copyright (c) 2015 Patrick Dumais

http://www.dumaisnet.ca

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include <map>

#define MAX_HTTP_DATA 1024

namespace Dumais
{
    namespace WebSocket
    {
        struct HTTPRequest
        {
            enum Status
            {
                Incomplete,
                Ready,
                Error
            } status;
            std::string type;
            std::string resource;
            std::map<std::string, std::string> headers;
            std::map<std::string,std::string> protocols;
        };

        class HTTPProtocolParser
        {
        private:
            char data[MAX_HTTP_DATA];
            size_t dataIndex;

            std::map<std::string,std::string> tokenize(char c, const std::string& subject);
        public:
            HTTPProtocolParser();
            ~HTTPProtocolParser();

            HTTPRequest parseIncoming(char* buffer, size_t size);
            std::string reject(int code, const std::string& response);
            std::string switchProtocol(const std::string& key, const std::string& protocol);

        };
    }
}
