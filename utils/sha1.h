#include "Hash.h"

namespace Dumais
{
    namespace Utils
    {
        class SHA1: public Hash
        {
        public:
            SHA1(const char* input, unsigned int inputSize);
        };

    }
}
