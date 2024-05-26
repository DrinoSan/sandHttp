#pragma once

// System Headers
#include <map>
#include <string>

namespace SandServer
{

//-----------------------------------------------------------------------------
class HTTPMessage_t
{
  public:
    virtual ~HTTPMessage_t() = default;

    void        setHeader( const std::string& name, const std::string& value );

    [[nodiscard]]
    std::string getHeader( const std::string& name ) const;
    inline void        setBody( const std::string& body_ )
    {
      body = body_;
    }

    [[nodiscard]]
    std::string getBody() const;

    void         printHeaders();
    virtual void printObject();

  protected:
    std::map<std::string, std::string> headers;
    std::string                        body;
};

//-----------------------------------------------------------------------------
class HTTPRequest_t : public HTTPMessage_t
{
  public:
     ~HTTPRequest_t() override = default;

    // Specific methods
    inline void setMethod( const std::string& method_ ) { method = method_; }
    
    [[nodiscard]]
    inline std::string getMethod() const { return method; };

    inline void        setURI( const std::string& uri_ ) { uri = uri_; }

    [[nodiscard]]
    inline std::string getURI() const { return uri; }

    inline void setVersion( const std::string& version_ )
    {
        version = version_;
    }

    [[nodiscard]]
    inline auto getVersion() const -> std::string { return version; }

    void printObject() override;

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
    virtual ~HTTPResponse_t() = default;

    // Specific methods
    void setStatusCode( int32_t statusCode_ ) { statusCode = statusCode_; }
    [[nodiscard]] inline int  getStatusCode() const { return statusCode; }
    void        setReasonPhrase( const std::string& reasonPhrase );
    [[nodiscard]] std::string getReasonPhrase() const;

    void printObject() override;

    static HTTPResponse_t notFound();

  private:
    int32_t statusCode;
};

};   // namespace SandServer
