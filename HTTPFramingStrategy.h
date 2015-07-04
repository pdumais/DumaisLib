#pragma once

#include "IFramingStrategy.h"

namespace Dumais{
namespace WebServer{
class HTTPFramingStrategy: public IFramingStrategy
{
protected:
    virtual int processData(char* buffer, size_t size, FullMessageInfo& info);
};

}
}
