#pragma once

#include <string>
#include <utility>

#define WS_CONTINUE 0x00
#define WS_TEXT 0x01
#define WS_BINARY 0x02
#define WS_CLOSE 0x08
#define WS_PING 0x09
#define WS_PONG 0x0A

namespace Dumais
{
    namespace WebSocket
    {
        struct WSHeader
        {
            unsigned char opcode:4;
            unsigned char reserved:3;
            unsigned char fin:1;
            unsigned char payloadSize:7;
            unsigned char mask:1;
        } __attribute__((packed));

        struct WSParsingResult
        {
            enum WSParsingStatus
            {
                Error,
                MessageReady,
                MessageIncomplete
            } status;
            size_t headerSize;
            size_t payloadSize;
            size_t dataRead;
            char* buffer;
        };

        class WSProtocolParser
        {
        private:
            char* data;
            size_t dataIndex;
            size_t currentBufferSize;
            size_t expectedLength;

        public:
            WSProtocolParser();
            ~WSProtocolParser();

            WSParsingResult parseWSMessage(char* buf, size_t size);
            std::pair<size_t,char*> sendData(int type, const char* buffer, size_t size);
            std::pair<size_t,char*> generateCloseMessage();
            void resetMessageBuffer();
        };
    }
}

