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

unsigned char* Hash::getDigest()
{
    return (unsigned char*)this->H;
}
