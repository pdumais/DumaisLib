#include "Base64.h"
#include <stdio.h>

#if __GNUC__ >= 4 and __GNUC_MINOR__ >= 3
#define MOVEBE(x) __builtin_bswap32(x);
#else
#define MOVEBE(x) ((x&0xff000000)>>24)|((x&0xff)<<24)|((x&0xff0000)>>8)|((x&0xff00)<<8)
#endif

char* Base64::digits = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Base64::encode(const char* data, unsigned int size)
{
    std::string str;
    unsigned int tmp;
    unsigned int i = 0;
    while (i<size)
    {
        tmp = MOVEBE(*((unsigned int*)&data[i]));
        if (size-i == 1)
        {
            str += digits[(tmp>>26)&0x3F];
            str += digits[(tmp>>20)&0x3F];
            str += "==";
            i++;
        }
        else if (size-i == 2)
        {
            str += digits[(tmp>>26)&0x3F];
            str += digits[(tmp>>20)&0x3F];
            str += digits[(tmp>>14)&0x3F];
            str += "=";
            i+=2;
        }
        else
        {
            str += digits[(tmp>>26)&0x3F];
            str += digits[(tmp>>20)&0x3F];
            str += digits[(tmp>>14)&0x3F];
            str += digits[(tmp>>8)&0x3F];
            i+=3;
        }   
    }
    
    return str;
}
