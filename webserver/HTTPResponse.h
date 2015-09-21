#pragma once
#include <string>

namespace Dumais{
namespace WebServer{
enum HTTPResponseCode{
    Continue,
    SwitchingProtocols,
    Processing,
    OK,
    Created,
    Accepted,
    NonAuthoritativeInformation,
    NoContent,
    ResetContent,
    PartialContent,
    MultiStatus,
    AlreadyReported,
    IMUsed,
    MultipleChoices,
    MovedPermanently,
    Found,
    SeeOther,
    NotModified,
    UseProxy,
    Reserved,
    TemporaryRedirect,
    PermanentRedirect,
    BadRequest,
    Unauthorized,
    PaymentRequired,
    Forbidden,
    NotFound,
    MethodNotAllowed,
    NotAcceptable,
    ProxyAuthenticationRequired,
    RequestTimeout,
    Conflict,
    Gone,
    LengthRequired,
    PreconditionFailed,
    RequestEntityTooLarge,
    RequestURITooLong,
    UnsupportedMediaType,
    RequestedRangeNotSatisfiable,
    ExpectationFailed,
    UnprocessableEntity,
    Locked,
    FailedDependency,
    UpgradeRequired,
    PreconditionRequired,
    TooManyRequests,
    RequestHeaderFieldsTooLarge,
    InternalServerError,
    NotImplemented,
    BadGateway,
    ServiceUnavailable,
    GatewayTimeout,
    HTTPVersionNotSupported,
    VariantAlsoNegotiates,
    InsufficientStorage,
    LoopDetected,
    NotExtended,
    NetworkAuthenticationRequired
};

// This is used to build a response. Not to parse one
class HTTPResponse
{
protected:
    friend class HTTPProtocol;
    HTTPResponseCode mResponseCode;
    std::string mContentType;    
    std::string mAuthHeader;
    std::string mHeader;
    std::string mCookie;
    bool mAuthenticationHack;
    bool mHeaderBuilt;
    const std::string& getCode(HTTPResponseCode code);
    bool buildHeader(size_t contentSize);

public:
    HTTPResponse();
    virtual ~HTTPResponse();
    virtual size_t getResponseChunk(char* buf, size_t startIndex, size_t max) = 0;
    virtual void setCookie(std::string cookie);

    void useAuthenticationHack();

};


}
}
