#include "WSProtocolParser.h"
#include "utils/Logging.h"
#include <string.h>

#define DEFAULT_DATA_SIZE 1024

using namespace Dumais::WebSocket;

WSProtocolParser::WSProtocolParser()
{
    this->currentBufferSize = DEFAULT_DATA_SIZE;
    this->data = new char[this->currentBufferSize];
    this->dataIndex = 0;
    this->expectedLength = sizeof(WSHeader);
}

WSProtocolParser::~WSProtocolParser()
{
    delete[] this->data;
}

void WSProtocolParser::resetMessageBuffer()
{
    this->dataIndex = 0;
    this->expectedLength = sizeof(WSHeader);
    if (this->currentBufferSize > DEFAULT_DATA_SIZE)
    {
        // To avoid using to much memory
        this->currentBufferSize = DEFAULT_DATA_SIZE;
        delete this->data;
        this->data = new char[this->currentBufferSize];
    }
}

WSParsingResult WSProtocolParser::parseWSMessage(char* buf, size_t bufSize)
{
    WSParsingResult result;
    result.status = WSParsingResult::Error;

    size_t toRead;
    if ((this->dataIndex+bufSize) > this->expectedLength)
    {
        toRead = this->expectedLength-this->dataIndex;
    }
    else
    {
        toRead = bufSize;
    }
    result.dataRead = toRead;


    // in this function, we are guaranteed that the size is not bigger than 1 message
    // but it can be smaller.
    memcpy((char*)&this->data[this->dataIndex], buf, toRead);
    this->dataIndex += toRead;
    // if we have a full message, process it. Otherwise continue accumulating the data
    if (this->dataIndex == this->expectedLength)
    {
        // At this point, we are guaranteed to have the minimum size for the header.
        // since this->dataIndex == this->expectedLength and xpectedLength cannot be smaller
        // than minimum header size
        WSHeader* header = (WSHeader*)&this->data[0];
        size_t headerSize = sizeof(WSHeader);

        if (header->mask)
        {
            headerSize += 4;
        }
        else
        {
            // must disconnect because this is not allowed as per RFC
            // "All frames sent from client to server have this bit set to 1." - RFC6455, section 5.2
            LOG("Client sent unmasked data.\r\n");
            return result;
        }

        if (header->payloadSize == 126)
        {
            headerSize += 2;
        }
        else if (header->payloadSize == 127)
        {
            headerSize += 8;
        }

        // if we were expecting the minimum size, update it to the real required size
        if (this->expectedLength == sizeof(WSHeader))
        {
            this->expectedLength = headerSize;
        }

        if (this->dataIndex == headerSize)
        {
            // We have the full header, now calculate the full payload size
            size_t payloadSize = header->payloadSize;
            if (payloadSize == 126) // 16bit value
            {
                payloadSize = ((size_t)this->data[2]<<8)|((size_t)this->data[3]);
            }
            else if (payloadSize == 127) // 64bit value
            {
                if (sizeof(size_t) != 8)
                {
                    LOG("64bit payload size not supported here\r\n");
                    return result;
                }
                //TODO: in assembly, this could be done in 2 instructions
                payloadSize = ((size_t)this->data[2]<<56)|
                              ((size_t)this->data[3]<<48)|
                              ((size_t)this->data[4]<<40)|
                              ((size_t)this->data[5]<<32)|
                              ((size_t)this->data[6]<<24)|
                              ((size_t)this->data[7]<<16)|
                              ((size_t)this->data[8]<<8)|
                              ((size_t)this->data[9]);
            }
            this->expectedLength = headerSize + payloadSize;

            // Reajust buffer size if bigger than what was previously allocated
            if (this->expectedLength > this->currentBufferSize)
            {
                //TODO: should check length to make sure we don't allocate a huge buffer.
                char *newBuffer = new char[this->expectedLength];
                memcpy(newBuffer,this->data,this->dataIndex);
                delete this->data;
                this->data = newBuffer;
                this->currentBufferSize = this->expectedLength;
            }
        }
        else if (this->expectedLength > headerSize)
        {
            result.payloadSize = this->expectedLength - headerSize;
            result.headerSize = headerSize;
            result.status = WSParsingResult::MessageReady;
            result.buffer = data;
            return result;
        }
    }
    result.status = WSParsingResult::MessageIncomplete;
    return result;
}


std::pair<size_t,char*> WSProtocolParser::sendData(int type, const char* buffer, size_t size)
{
    std::pair<size_t,char*> ret;
    ret.second=0;

    char* packet = new char[size+sizeof(WSHeader)+8]; // make it bigger in case we need 8 bytes for payloadSize
    size_t headerSize = sizeof(WSHeader);
    WSHeader* header = (WSHeader*)packet;
    header->reserved = 0;
    header->mask = 0;
    header->fin = 1; // we do not support fragmentation yet
    header->opcode = type;
    if (size > 125)
    {
        if (size > 65535)
        {
            //headerSize += 8;
            //header->payloadSize = 127; //rfc6455 section 5.2
            LOG("64 bit size is not supporte yet for TX\r\n");
            return ret;
        }
        else
        {
            headerSize += 2;
            header->payloadSize = 126; //rfc6455 section 5.2
            packet[2] = (char)(size>>8);
            packet[3] = (char)(size&0xFF);
        }
    }
    else
    {
        header->payloadSize = size;
    }
    memcpy((char*)&packet[headerSize],buffer,size);
    ret.first = size+headerSize;
    ret.second = packet;
    return ret;
}

std::pair<size_t,char*> WSProtocolParser::generateCloseMessage()
{
    //TODO: must support close reasons.

    std::pair<size_t,char*> ret;
    ret.first = sizeof(WSHeader);
    ret.second = new char[ret.first];
    WSHeader* header = (WSHeader*)ret.second;
    header->reserved = 0;
    header->mask = 0;
    header->fin = 1;
    header->opcode = WS_CLOSE;
    header->payloadSize = 0;

    return ret;
}

