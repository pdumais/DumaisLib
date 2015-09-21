#pragma once

#include "ISocket.h"
#include <string>

#define CIPHER_LIST "AES256-SHA"        // FIPS-140-2 approved
#define SSL_SERVER_METHOD TLSv1_server_method()
#define SSL_CLIENT_METHOD TLSv1_client_method()

#ifdef USING_OPENSSL
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#else
#define SSL_CTX void
#define SSL void
#endif
namespace Dumais{
namespace WebServer{

class SecureSocket: public ISocket
{
private:
    void initialize();
    bool error(int err);
    bool clean();
    void initClient(SSL_CTX* context);
    
    static bool isInitialized;

    SSL* sslHandle;
    SSL_CTX* sslContext;
    bool client;

public:
    SecureSocket(int socket);

    virtual void close();
    virtual ISocket* accept();
    virtual int read(char* buffer, size_t size);
    virtual int send(char* buffer, size_t size);

    void initServer(const std::string& certificatePath, const std::string& privateKeyPath);
};

}
}


