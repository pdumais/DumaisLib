#include "HTTPResponse.h"
#include <sstream>

using namespace Dumais::WebServer;

std::string responseCodesString[] = {
"100 Continue",
"101 SwitchingProtocols",
"102 Processing",
"200 OK",
"201 Created",
"202 Accepted",
"203 NonAuthoritativeInformation",
"204 NoContent",
"205 ResetContent",
"206 PartialContent",
"207 Multi-Status",
"208 AlreadyReported",
"226 IMUsed",
"300 MultipleChoices",
"301 MovedPermanently",
"302 Found",
"303 SeeOther",
"304 NotModified",
"305 UseProxy",
"306 Reserved",
"307 TemporaryRedirect",
"308 PermanentRedirect",
"400 BadRequest",
"401 Unauthorized",
"402 PaymentRequired",
"403 Forbidden",
"404 NotFound",
"405 MethodNotAllowed",
"406 NotAcceptable",
"407 ProxyAuthenticationRequired",
"408 RequestTimeout",
"409 Conflict",
"410 Gone",
"411 LengthRequired",
"412 PreconditionFailed",
"413 RequestEntityTooLarge",
"414 Request-URITooLong",
"415 UnsupportedMediaType",
"416 RequestedRangeNotSatisfiable",
"417 ExpectationFailed",
"422 UnprocessableEntity",
"423 Locked",
"424 FailedDependency",
"426 UpgradeRequired",
"428 PreconditionRequired",
"429 TooManyRequests",
"431 RequestHeaderFieldsTooLarge",
"500 InternalServerError",
"501 NotImplemented",
"502 BadGateway",
"503 ServiceUnavailable",
"504 GatewayTimeout",
"505 HTTPVersionNotSupported",
"506 VariantAlsoNegotiates(Experimental)",
"507 InsufficientStorage",
"508 LoopDetected",
"510 NotExtended",
"511 NetworkAuthenticationRequired"
};

HTTPResponse::HTTPResponse()
{
    mHeaderBuilt = false;
    mAuthenticationHack = false;
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::useAuthenticationHack()
{
    // When using a XMLHttpRequest on the client side, sending a 401 with WWW-authenticate
    // will trigger a password prompt on many browser. This might be undesireable if the client
    // side code wants to take care of it. So the hack is to send a X-WWW-authenticate instead
    mAuthenticationHack = true;
}

const std::string& HTTPResponse::getCode(HTTPResponseCode code)
{
    return responseCodesString[code];
}


//TODO: should set one cookie at a time instead of the full line
void HTTPResponse::setCookie(std::string cookie)
{
    mCookie = cookie;
}

bool HTTPResponse::buildHeader(size_t contentSize)
{
    if (mHeaderBuilt) return false;

    std::stringstream ss;
    ss <<"HTTP/1.1 "<< getCode(mResponseCode) << "\r\n";
    ss <<"Content-Length: " << contentSize << "\r\n";
    if (mAuthHeader!="")
    {   
        if (mAuthenticationHack) ss << "X-WWW-Authenticate: "; else ss << "WWW-Authenticate: ";
        ss << mAuthHeader << "\r\n";
    }
    if (mContentType!="") ss <<"Content-Type: " << mContentType <<"; charset=UTF-8\r\n";
    if (mCookie!="") ss <<"Set-Cookie: " << mCookie << "\r\n";
    ss <<"Connection: Close\r\n";
    //ss <<"Access-Control-Allow-Method: GET\r\n"; // to allow cross-site scripting
    ss <<"Access-Control-Allow-Origin: *\r\n"; // to allow cross-site scripting
    //ss <<"Access-Control-Allow-Credentials: true\r\n"; // to allow cross-site scripting
//    ss <<"Access-Control-Expose-Headers: Authorization,  WWW-Authenticate, Set-Cookie\r\n"; // to allow cross-site scripting
  //  ss <<"Access-Control-Allow-Headers: authorization, x-dhassession\r\n";
    ss << "\r\n";

    mHeader = ss.str();

    mHeaderBuilt = true;
    return true;
}

