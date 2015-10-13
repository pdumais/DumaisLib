#include "Hash.h"

using namespace Dumais::Utils;

Hash::Hash(unsigned int hashSize, const char* input, unsigned int inputSize)
{
    this->mHashSize = hashSize;
    this->H = new unsigned int[hashSize/4];
}

Hash::~Hash()
{
    if (this->H) delete this->H;
    this->H = 0;
}

std::string Hash::toHex()
{
    unsigned char* buf = (unsigned char*)this->H;

    std::string output((this->mHashSize*2),0);
    for (int i = 0; i < this->mHashSize; i++)
    {
        output[(i*2)+1] = mDigits[(buf[(i)] & 0x0F)];
        output[(i*2)] = mDigits[(buf[(i)] & 0xF0)>>4];
    }
//output[this->mHashSize*2]=0;
    return output;
}

