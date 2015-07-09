#include "SecureSocket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace Dumais::WebServer;

bool SecureSocket::isInitialized = false;

SecureSocket::SecureSocket(int socket)
{
#ifdef USING_OPENSSL
    mSocket = socket;
    sslHandle = 0;
    sslContext = 0;
    client = false;
    initialize();
#endif
}

void SecureSocket::initialize()
{
#ifdef USING_OPENSSL
    if (SecureSocket::isInitialized) return;

    SSL_library_init ();
    OpenSSL_add_all_algorithms();
    SecureSocket::isInitialized = true;
#endif
}

bool SecureSocket::error(int err)
{
#ifdef USING_OPENSSL
    return (err == X509_V_OK);
#else
    return false;
#endif
}

bool SecureSocket::clean()
{
#ifdef USING_OPENSSL
    if (this->sslHandle)
    {
        SSL_shutdown(this->sslHandle);
        SSL_free(this->sslHandle);
        this->sslHandle = 0;
    }

    if (this->sslContext && !this->client)
    {
        SSL_CTX_free(this->sslContext);
        this->sslContext = 0;
    }
    return true;
#else
    return false;
#endif
}

void SecureSocket::close()
{
#ifdef USING_OPENSSL
    this->clean();
    ::close(mSocket);
#endif
}

ISocket* SecureSocket::accept()
{
#ifdef USING_OPENSSL
    int s = ::accept(this->mSocket,0,0);
    if (s == -1) return 0;

    SecureSocket *sock = new SecureSocket(s);
    sock->initClient(this->sslContext);
    return sock;
#else
    return 0;
#endif
}

int SecureSocket::read(char* buffer, size_t size)
{
#ifdef USING_OPENSSL
    if (!this->sslHandle) return 0;
    return SSL_read(this->sslHandle, buffer, size);
#else
    return 0;
#endif
}

int SecureSocket::send(char* buffer, size_t size)
{
#ifdef USING_OPENSSL
    if (!this->sslHandle) return 0;
    return SSL_write(this->sslHandle, buffer, size);
#else
    return 0;
#endif
}

    
void SecureSocket::initClient(SSL_CTX* context)
{
#ifdef USING_OPENSSL
    this->client = true;
    this->sslContext = context;
    this->sslHandle = 0;

    this->sslHandle = SSL_new(this->sslContext);
    if (!this->sslHandle) { this->clean(); return; }
    SSL_set_verify(this->sslHandle,SSL_VERIFY_NONE,0);
    if (!SSL_set_fd(this->sslHandle, this->mSocket)) { this->clean(); return; }
    if (SSL_accept(this->sslHandle)<=0) { this->clean(); return; }
#endif
}

void SecureSocket::initServer(const std::string& certificatePath, const std::string& privateKeyPath)
{
#ifdef USING_OPENSSL
    this->sslContext = 0;
    this->sslHandle = 0;

    this->sslContext = SSL_CTX_new(SSL_SERVER_METHOD);
    if (!this->sslContext) {this->clean(); return;}
    SSL_CTX_set_cipher_list(this->sslContext, CIPHER_LIST);
    if (SSL_CTX_use_certificate_file(this->sslContext,certificatePath.c_str() ,SSL_FILETYPE_PEM)<=0) {this->clean(); return;}
    if (SSL_CTX_use_PrivateKey_file(this->sslContext,privateKeyPath.c_str(), SSL_FILETYPE_PEM)<=0) {this->clean(); return;}
#endif
}
