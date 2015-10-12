#include "sha256.h"
#include <string.h>
#include <stdio.h>

#if __GNUC__ >= 4 and __GNUC_MINOR__ >= 3
#define MOVEBE(x,y) y= __builtin_bswap32(x);
#else
#define MOVEBE(x,y) y= ((x&0xff000000)>>24)|((x&0xff)<<24)|((x&0xff0000)>>8)|((x&0xff00)<<8)
#endif

#define ROR(x,shift) ((x>>shift)|(x<<(32-shift))) // could be faster with real ror instruction

char* digits = "0123456789abcdef";

// The following are constants defined in the specs
unsigned int CACHEDH[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
unsigned int K[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
                    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
                    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
                    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
                    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
                    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
                    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
                    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};


// function to vonert the hash to a ascii hex representation of the hash.
void tohex(unsigned char* input, unsigned int size, char* output)
{
    for (int i = 0; i < size; i++)
    {
        output[(i*2)+1] = digits[(input[(i)] & 0x0F)];
        output[(i*2)] = digits[(input[(i)] & 0xF0)>>4];
    }
}

// The actual work
void Dumais::Utils::sha256(char* input, unsigned int size, char* output)
{
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
    unsigned int H[8];

    for (i=0;i<8;i++) H[i]=CACHEDH[i];

    unsigned int size2 = size;
    unsigned int size3 = 0;
    unsigned int* words = new unsigned int[16];
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
    tohex((unsigned char*)&H[0],32,output);
}



