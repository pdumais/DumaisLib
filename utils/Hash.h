#pragma once
#include <string>

namespace Dumais
{
    namespace Utils
    {
        class Hash
        {
        private:
            unsigned int mHashSize;
            char* mDigits = "0123456789abcdef";

        protected:
            unsigned int* H;

        public:
            Hash(unsigned int hashSize, const char* input, unsigned int inputSize);
            virtual ~Hash();
            std::string toHex();
        };
    }
}
