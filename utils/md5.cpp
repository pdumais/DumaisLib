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
#include "md5.h"
#include <string.h>
#include <stdio.h>

#define ROL(x,shift) ((x<<shift)|(x>>(32-shift))) // could be faster with real rol instruction

using namespace Dumais::Utils;

MD5::MD5(const char* input, unsigned int size): Hash(16,input,size)
{
    H[0] = 0x67452301;
    H[1] = 0xefcdab89;
    H[2] = 0x98badcfe;
    H[3] = 0x10325476;

std::string st;
st.assign(input,size);
printf("$$$$$$$$$$$$$$$$ [%s]\r\n",st.c_str());

    unsigned long i;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int f;
    unsigned int g;
    unsigned int tmp;

    unsigned int size2 = size;
    unsigned int size3 = 0;
    unsigned int words[16];

    unsigned long numChunks = (size+1+8+63) >> 6;// div by 512bit chunks (div by 64bytes)
    unsigned int frameIndexForSize = numChunks-1;

    unsigned char trail = 0x80;
    for (unsigned long chunkIndex = 0; chunkIndex < numChunks; chunkIndex++)  // for each 512bit chunk
    {
        if (size2>=64)
        {
            size3=64;
            memcpy((void*)words,(void*)input,64);
        }
        else
        {
            size3=size2;
            char* cbuf = (char*)words;
            if (size3) memcpy((void*)words,(void*)input,size3);
            memset((char*)&cbuf[size3],0,64-size3);
            ((unsigned char*)words)[size3] = trail;
            trail = 0;
        }

        if (chunkIndex == frameIndexForSize) // do this only a the end of the last frame
        {
            ((unsigned long*)words)[7] = (((unsigned long)size)<<3);
        }
        input+=64;
        size2-=size3;
        unsigned int* chunkWords = words;

        a=H[0];b=H[1];c=H[2];d=H[3];

        for (i=0;i<64;i++)
        {
            if (i < 16)
            {
                f = (b&c)|(~b&d);
                g = i;
            }
            else if (i < 32)
            {
                f = (d&b)|(~d&c);
                g = (5*i +1)&0x0F;
            }
            else if (i < 48)
            {
                f = b^c^d;
                g = (3*i +5)&0x0F;
            }
            else if (i < 64)
            {
                f = c^(b|~d);
                g = (7*i)&0x0F;
            }
            tmp = d;
            d = c;
            c = b;
            b = b+ROL(a+f+K[i]+chunkWords[g],s[i]);
            a = tmp;
            
        }

        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;
    }

std::string st1 = toHex();
printf("$$$$$$$$$$$$$$$$ MD5 [%s]\r\n",st1.c_str());
}



