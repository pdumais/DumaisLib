#include "HTTPRequest.h"
#include <algorithm>
#include <stdio.h>
#include "regex.h"
#include <regex>

using namespace Dumais::WebServer;

HTTPRequest::HTTPRequest(char* buffer, size_t size)
{
    mBuffer = buffer;
    mSize = size;

    size_t pos = 3;
    char *dest = (char*)&mBuffer[pos];
    while (pos < mSize)
    {   
        if (*((unsigned int*)(dest-3)) == 0x0A0D0A0D) break;
        dest++;
        pos++;    
    }

    mHeader.assign(mBuffer,pos);

}

HTTPRequest::~HTTPRequest()
{
}

std::string HTTPRequest::getHeader(const std::string& header)
{
    std::regex r(header+": (.*)",std::regex_constants::icase);
    std::smatch matches;
    if (std::regex_search(mHeader,matches,r))
    {
        return matches[1].str();
    }
    return "";
}


bool HTTPRequest::accepts(const std::string& accept)
{
    std::string a = this->getHeader("Accept");
    return (a.find(accept)!=std::string::npos);
}

std::string HTTPRequest::getURL()
{
    std::regex r("^(.*) (.*) (.*)",std::regex_constants::icase);
    std::smatch matches;
    if (std::regex_search(mHeader,matches,r))
    {
        return matches[2].str();
    }
}

std::string HTTPRequest::getMethod()
{
    std::regex r("^(.*) (.*) (.*)",std::regex_constants::icase);
    std::smatch matches;
    if (std::regex_search(mHeader,matches,r))
    {
        return matches[1].str();
    }
}


HTTPAuth HTTPRequest::getAuthorization()
{
    HTTPAuth auth;
    auth.nonce = "";

    std::string authLine = this->getHeader("authorization");
    std::map<std::string,std::string> fields = getAuthFields(authLine);
    if (fields.find("nonce") != fields.end()) auth.nonce = fields["nonce"];
    if (fields.find("digest username") != fields.end()) auth.username = fields["digest username"];
    if (fields.find("response") != fields.end()) auth.response = fields["response"];
    if (fields.find("uri") != fields.end()) auth.uri = fields["uri"];
    return auth;
}

bool HTTPRequest::wantsAuthenticationHack()
{
    return (this->getHeader("x-needauthenticationhack")!="");
}

std::string HTTPRequest::getCookie(const std::string& name)
{
    std::regex r(name+"=([a-zA-Z0-9\\. ]*);?");
    std::smatch matches;

    std::string cookieLine = this->getHeader("cookie");
    if (cookieLine == "") return "";

    std::regex_search(cookieLine,matches,r);
    return matches[1].str();
}

std::map<std::string,std::string> HTTPRequest::getAuthFields(std::string st)
{
    std::map<std::string,std::string> ret;
    std::regex r(" ?([a-zA-Z0-9 ]*)=\"([^\"]*)\"");
    std::smatch matches;

    std::sregex_iterator it(st.begin(), st.end(), r);
    while(it != std::sregex_iterator())
    {
        std::smatch matches = *it;
        std::string key = matches[1].str();
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ret[key] = matches[2].str();
        it++;
    }
    return ret;
}

