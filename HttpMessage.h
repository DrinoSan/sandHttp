#pragma once

// System Headers
#include <map>
#include <optional>
#include <string>
#include <vector>

// Project Headers
#include "HttpStatusCodes.h"
#include "Log.h"
#include "SandMethod.h"

namespace SandServer
{

//-----------------------------------------------------------------------------
class HTTPMessage_t
{
 public:
   //-----------------------------------------------------------------------------
   HTTPMessage_t() = default;

   //-----------------------------------------------------------------------------
   HTTPMessage_t( const HTTPMessage_t& other )            = default;
   HTTPMessage_t& operator=( const HTTPMessage_t& other ) = default;

   //-----------------------------------------------------------------------------
   HTTPMessage_t( HTTPMessage_t&& other ) noexcept            = default;
   HTTPMessage_t& operator=( HTTPMessage_t&& other ) noexcept = default;

   //-----------------------------------------------------------------------------
   virtual ~HTTPMessage_t() = default;

   //-----------------------------------------------------------------------------
   void setHeader( const std::string& name, const std::string& value );
   //-----------------------------------------------------------------------------
   std::string stringifyHeaders();

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   std::optional<std::string> getHeader( const std::string& name ) const;

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   bool isHeaderSet( const std::string& name ) const
   {
      return headers.find( name ) != headers.end();
   }

   //-----------------------------------------------------------------------------
   inline void setBody( const std::string& body_ ) { body = body_; }

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   inline std::string getBody()
   {
      return body;
   }

   //-----------------------------------------------------------------------------
   [[nodiscard]] inline std::string getBody() const { return body; }

   //-----------------------------------------------------------------------------
   void printHeaders();
   //-----------------------------------------------------------------------------
   virtual void printObject();

 protected:
   std::map<std::string, std::string> headers;
   std::string                        body;
};

//-----------------------------------------------------------------------------
class HTTPRequest_t : public HTTPMessage_t
{
 public:
   //-----------------------------------------------------------------------------
   HTTPRequest_t() = default;

   //-----------------------------------------------------------------------------
   HTTPRequest_t( const HTTPRequest_t& other )            = default;
   HTTPRequest_t& operator=( const HTTPRequest_t& other ) = default;

   //-----------------------------------------------------------------------------
   HTTPRequest_t( HTTPRequest_t&& other ) noexcept            = default;
   HTTPRequest_t& operator=( HTTPRequest_t&& other ) noexcept = default;

   //-----------------------------------------------------------------------------
   ~HTTPRequest_t() override = default;

   //-----------------------------------------------------------------------------
   // Specific methods
   inline void setMethod( const std::string& method_ ) { method = method_; }

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   inline SAND_METHOD getMethod() const
   {
      return httpMethod::stringToMethod( method );
   };

   //-----------------------------------------------------------------------------
   inline void setURI( const std::string& uri_ ) { uri = uri_; }

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   inline std::string getURI() const
   {
      return uri;
   }

   //-----------------------------------------------------------------------------
   inline void setVersion( const std::string& version_ ) { version = version_; }

   //-----------------------------------------------------------------------------
   inline void setUrlParts( const std::vector<std::string>& urlParts_ )
   {
      urlParts = urlParts_;
   }

   //-----------------------------------------------------------------------------
   [[nodiscard]] inline std::vector<std::string> getUrlParts() const
   {
      return urlParts;
   }

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   inline auto getVersion() const -> std::string
   {
      return version;
   }

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   std::string pathValue( const std::string& value ) const;

   //-----------------------------------------------------------------------------
   void printObject() override;

 public:
   std::map<std::string, std::string> query;
   // std::map<std::string, std::string> pathParameters;

 private:
   std::string              method;
   std::string              uri;
   std::string              version;
   std::vector<std::string> urlParts;
};

//-----------------------------------------------------------------------------
class HTTPResponse_t : public HTTPMessage_t
{
 public:
   //-----------------------------------------------------------------------------
   HTTPResponse_t()
       : statusCode{ StatusCode::OK },
         reasonPhrase{ SandServer::reasonPhrase( statusCode ) },
         httpVersion{ "HTTP/1.1" }
   {
   }

   //-----------------------------------------------------------------------------
   explicit HTTPResponse_t( StatusCode statusCode_ )
       : statusCode{ statusCode_ },
         reasonPhrase{ SandServer::reasonPhrase( statusCode ) },
         httpVersion{ "HTTP/1.1" }
   {
   }

   //-----------------------------------------------------------------------------
   HTTPResponse_t( const HTTPResponse_t& other )            = default;
   HTTPResponse_t& operator=( const HTTPResponse_t& other ) = default;

   //-----------------------------------------------------------------------------
   HTTPResponse_t( HTTPResponse_t&& other ) noexcept            = default;
   HTTPResponse_t& operator=( HTTPResponse_t&& other ) noexcept = default;

   //-----------------------------------------------------------------------------
   ~HTTPResponse_t() override = default;

   //-----------------------------------------------------------------------------
   void setStatusCode( int32_t statusCode_ ) { statusCode = statusCode_; }

   //-----------------------------------------------------------------------------
   [[nodiscard]] inline int getStatusCode() const { return statusCode; }

   //-----------------------------------------------------------------------------
   void setReasonPhrase( const std::string& reasonPhrase_ )
   {
      reasonPhrase = reasonPhrase_;
   }

   //-----------------------------------------------------------------------------
   void setReasonPhraseByStatusCode( const StatusCode& statusCode_ )
   {
      reasonPhrase = SandServer::reasonPhrase( statusCode_ );
   }

   //-----------------------------------------------------------------------------
   [[nodiscard]] std::string getReasonPhrase() const;

   //-----------------------------------------------------------------------------
   void prepareResponse();

   //-----------------------------------------------------------------------------
   void printObject() override;

   //-----------------------------------------------------------------------------
   void notFound();

   //-----------------------------------------------------------------------------
   [[nodiscard]]
   std::string stringifyHeaders();

 private:
   // Specific methods
   [[nodiscard]]
   inline std::string setStatusLine()
   {
      return httpVersion + " " + std::to_string( statusCode ) + " " +
             reasonPhrase + "\r\n";
   }

 private:
   int32_t     statusCode{};
   std::string reasonPhrase{};
   std::string httpVersion{};
};

};   // namespace SandServer
