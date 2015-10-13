#include "sha1.h"
#include <string.h>
#include <stdio.h>

#if __GNUC__ >= 4 and __GNUC_MINOR__ >= 3
#define MOVEBE(x,y) y= __builtin_bswap32(x);
#else
#define MOVEBE(x,y) y= ((x&0xff000000)>>24)|((x&0xff)<<24)|((x&0xff0000)>>8)|((x&0xff00)<<8)
#endif

#define ROL(x,shift) ((x<<shift)|(x>>(32-shift))) // could be faster with real rol instruction

using namespace Dumais::Utils;

SHA1::SHA1(const char* input, unsigned int size): Hash(20,input,size)
{
    this->H[0] = 0x67452301;
    this->H[1] = 0xEFCDAB89;
    this->H[2] = 0x98BADCFE;
    this->H[3] = 0x10325476;
    this->H[4] = 0xC3D2E1F0;

    unsigned long i;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int f;
    unsigned int k;
    unsigned int t1;
    unsigned int t2;
    unsigned int ch;
    unsigned int maj;

    unsigned int size2 = size;
    unsigned int size3 = 0;
    unsigned int words[16];
    unsigned long numChunks = (size+1+8+63) >> 6;// div by 512bit chunks (div by 64bytes)
    unsigned int frameIndexForSize = numChunks-1;

    unsigned int w[80];
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
        for  (i=16;i<80;i++) w[i] = ROL((w[i-3]^w[i-8]^w[i-14]^w[i-16]),1);

        a=H[0];b=H[1];c=H[2];d=H[3];e=H[4];

        
        for (i=0;i<80;i++)
        {
            if (i<20)
            {
                f = (b&c)|((~b)&d);
                k = 0x5a827999;
            }
            else if (i<40)
            {
                f = b^c^d;
                k = 0x6ed9eba1;
            }
            else if (i<60)
            {
                f = (b&c)|(b&d)|(c&d); 
                k = 0x8f1bbcdc;
            }
            else
            {
                f = b^c^d;
                k = 0xca62c1d6;
            }

            k=ROL(a,5)+f+e+k+w[i];
            e=d;
            d=c;
            c=ROL(b,30);
            b=a;
            a=k;
        }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;


    }

    for (i=0;i<5;i++) MOVEBE(H[i],H[i]);
}



