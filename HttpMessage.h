#pragma once

// System Headers
#include <map>
#include <string>

namespace SandServer
{

// TODO: Should this be virtual?????
// TODO: Add method to log all fields of object. Make it virtual and in child
// PrintMe method call BASE::PrintMe
//-----------------------------------------------------------------------------
class HTTPMessage_t
{
  public:
    HTTPMessage_t() = default;

    void        setHeader( const std::string& name, const std::string& value );
    std::string getHeader( const std::string& name ) const;
    void        printHeaders();
    void        setBody( const std::string& body );
    std::string getBody() const;

  protected:
    std::map<std::string, std::string> headers;
    std::string                        body;
};

//-----------------------------------------------------------------------------
class HTTPRequest_t : public HTTPMessage_t
{
  public:
    HTTPRequest_t() = default;

    // Specific methods
    inline void setMethod( const std::string& method_ ) { method = method_; }
    inline std::string getMethod() const { return method; };

    inline void        setURI( const std::string& uri_ ) { uri = uri_; }
    inline std::string getURI() const { return uri; }

    inline void setVersion( const std::string& version_ )
    {
        version = version_;
    }
    inline std::string getVersion() const { return version; }

  private:
    std::string method;
    std::string uri;
    std::string version;
};

//-----------------------------------------------------------------------------
class HTTPResponse_t : public HTTPMessage_t
{
  public:
    HTTPResponse_t() = default;

    // Specific methods
    void        setStatusCode( int statusCode );
    int         getStatusCode() const;
    void        setReasonPhrase( const std::string& reasonPhrase );
    std::string getReasonPhrase() const;
};

};   // namespace SandServer
