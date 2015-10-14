#pragma once
#include <string>

class Base64
{
private:
    static char* digits;
public:
    static std::string encode(const char* data, unsigned int size);
};
