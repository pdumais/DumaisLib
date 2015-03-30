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
#include "HTTPProtocolParser.h"
#include "Logging.h"
#include "sha1.h"
#include "base64.h"
#include <sstream>
#include <algorithm>
#include <string.h>

#define MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

using namespace Dumais::WebSocket;

HTTPProtocolParser::HTTPProtocolParser()
{
    this->dataIndex = 0;
}

HTTPProtocolParser::~HTTPProtocolParser()
{
}

HTTPRequest HTTPProtocolParser::parseIncoming(char* buffer, size_t size)
{
    HTTPRequest req;
    req.status = HTTPRequest::Incomplete;

    // Since this is a websocket server, we do not expect large requests.
    // Although it would be possible to receive a GET with a body, we will not anticipate this
    // and we will return an error if the request is too large.
    size_t tmp = this->dataIndex + size;
    if (tmp >= MAX_HTTP_DATA)
    {
        req.status =  HTTPRequest::Error; //TODO: should instruct caller to send a 431
        return req;
    }
    memcpy((char*)&this->data[this->dataIndex], buffer, size);
    this->dataIndex = tmp;
    buffer = (char*)&this->data[0];
    size = this->dataIndex;

    // if buffer size is smaller than 4, it means we don't even have a "\r\n\r\n ", 
    // so no full header. So return now.
    if (size <4) return req;

    std::string str;
    str.assign(buffer,size);

    size_t endOfHeader = str.find("\r\n\r\n");
    if (endOfHeader == std::string::npos)
    {
        //TODO: if there is a body, we should read it.
        return req; 
    }
    endOfHeader+=4;
    // At this point, we assume we have a full request.

    size_t space1 = str.find(" ",0);
    if (space1 == std::string::npos)
    {
        req.status =  HTTPRequest::Error;
        return req;
    }
    req.type = str.substr(0,space1);
    
    // get the resource
    size_t space2 = str.find(" ",space1+1);
    if (space2 == std::string::npos)
    {
        req.status =  HTTPRequest::Error;
        return req;
    }
    req.resource = str.substr(space1+1,space2-space1-1);

    size_t eol = str.find("\n",0);
    if (eol == std::string::npos)
    {
        req.status =  HTTPRequest::Error;
        return req;
    }

    bool isHeader = true;
    std::string header;
    std::string value;
    for (size_t i = eol+1; i < endOfHeader; i++)
    {
        if (buffer[i]=='\n')
        {
            if (header!= "" && value != "")
            {
                std::transform(header.begin(), header.end(), header.begin(), ::tolower);
                req.headers[header] = value;
                header = "";
                value = "";
            }
            isHeader = true;
        }
        else if (buffer[i]=='\r')
        {
        }
        else
        {
            if (isHeader)
            {
                if (buffer[i] == ':')
                {
                    isHeader = false;
                }
                else
                {
                    header+=buffer[i];
                }
            }
            else
            {
                if (value.size() == 0 && buffer[i]==' ')
                {
                    //ignore leading spaces
                }
                else
                {
                    value+=buffer[i];
                }
            }
        }
    }

    this->dataIndex = 0;
    req.status = HTTPRequest::Ready;
    return req;
}

std::string HTTPProtocolParser::reject(int code, const std::string& response)
{
    std::stringstream ss;
    ss << "HTTP/1.1 " << code << " " << response << "\r\nContent-type: text/html\r\nAccess-Control-Allow-Origin: *\r\nContent-length: 0\r\n\r\n";
    return ss.str();
}

std::string HTTPProtocolParser::switchProtocol(const std::string& key)
{
    std::string accept = key+MAGIC;

    SHA1 sha;
    sha.addBytes(accept.c_str(),accept.size());
    accept = base64_encode(sha.getDigest(),20);

    std::stringstream ss;
    ss << "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " << accept << "\r\n\r\n";

    return ss.str();
}

