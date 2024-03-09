// System Headers
#include <map>
#include <string>

class HTTPMessage
{
  public:
    HTTPMessage();

    void        setHeader( const std::string& name, const std::string& value );
    std::string getHeader( const std::string& name ) const;
    void        setBody( const std::string& body );
    std::string getBody() const;

  protected:
    std::map<std::string, std::string> headers;
    std::string                        body;
};

class HTTPRequest : public HTTPMessage
{
  public:
    HTTPRequest();

    // Specific methods
    void        setMethod( const std::string& method );
    std::string getMethod() const;
    void        setURI( const std::string& uri );
    std::string getURI() const;
};

class HTTPResponse : public HTTPMessage
{
  public:
    HTTPResponse();

    // Specific methods
    void        setStatusCode( int statusCode );
    int         getStatusCode() const;
    void        setReasonPhrase( const std::string& reasonPhrase );
    std::string getReasonPhrase() const;
};
