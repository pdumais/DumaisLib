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
#include "sha256.h"
#include <string.h>
#include <stdio.h>

#if __GNUC__ >= 4 and __GNUC_MINOR__ >= 3
#define MOVEBE(x,y) y= __builtin_bswap32(x);
#else
#define MOVEBE(x,y) y= ((x&0xff000000)>>24)|((x&0xff)<<24)|((x&0xff0000)>>8)|((x&0xff00)<<8)
#endif

#define ROR(x,shift) ((x>>shift)|(x<<(32-shift))) // could be faster with real ror instruction

using namespace Dumais::Utils;

SHA256::SHA256(const char* input, unsigned int size): Hash(32,input,size)
{
    this->H[0] = 0x6a09e667;
    this->H[1] = 0xbb67ae85;
    this->H[2] = 0x3c6ef372;
    this->H[3] = 0xa54ff53a;
    this->H[4] = 0x510e527f;
    this->H[5] = 0x9b05688c;
    this->H[6] = 0x1f83d9ab;
    this->H[7] = 0x5be0cd19;

    unsigned long i;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int f;
    unsigned int g;
    unsigned int h;
    unsigned int t1;
    unsigned int t2;
    unsigned int ch;
    unsigned int maj;


    unsigned int size2 = size;
    unsigned int size3 = 0;
    unsigned int words[16];

    unsigned long numChunks = (size+1+8+63) >> 6;// div by 512bit chunks (div by 64bytes)
    unsigned int frameIndexForSize = numChunks-1;

    unsigned int w[64];
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
            unsigned long bigEndianSize = ((unsigned long)size)<<3; // size in bits
            for (i=0;i<8;i++)
            {
                ((unsigned char*)words)[63-i] = (unsigned char)(bigEndianSize & 0xFF);
                bigEndianSize = bigEndianSize >> 8;
            }
        }
        input+=64;
        size2-=size3;
        unsigned int* chunkWords = words;

        for (i=0;i<16;i++) MOVEBE(chunkWords[i],w[i]);
        for (i=16;i<64;i++)
        {
            unsigned int s0 = ROR(w[i-15],7)^ROR(w[i-15],18)^(w[i-15] >> 3);
            unsigned int s1 = ROR(w[i-2],17)^ROR(w[i-2],19)^(w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }

        a=H[0];b=H[1];c=H[2];d=H[3];e=H[4];f=H[5];g=H[6];h=H[7];

        for (i=0;i<64;i++)
        {
            unsigned int s1 = ROR(e,6)^ROR(e,11)^ROR(e,25);
            ch = (e&f)^((~e)&g);
            t1 = h+s1+ch+K[i]+w[i];
            unsigned int s0 = ROR(a,2)^ROR(a,13)^ROR(a,22);
            maj = (a&b)^(a&c)^(b&c);
            t2 = s0 + maj;
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;


    }
    for (i=0;i<8;i++) MOVEBE(H[i],H[i]);
}



