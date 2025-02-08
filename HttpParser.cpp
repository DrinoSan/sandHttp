// System Headers

// Project Headers
#include "HttpParser.h"

namespace SandServer
{
//-----------------------------------------------------------------------------
HTTPRequest_t HttpParser_t::parseRequest( const std::string& rawMessage )
{
   HTTPRequest_t request;
   size_t        pos = 0;
   size_t        end = rawMessage.find( "\r\n\r\n" );

   if ( end == std::string::npos )
   {
      throw std::runtime_error( "Invalid HTTP request format." );
   }

   // Parse request line
   size_t methodEnd = rawMessage.find( ' ', pos );
   request.setMethod( rawMessage.substr( pos, methodEnd - pos ) );

   pos           = methodEnd + 1;
   size_t uriEnd = rawMessage.find( ' ', pos );
   request.setURI( rawMessage.substr( pos, uriEnd - pos ) );

   pos               = uriEnd + 1;
   size_t versionEnd = rawMessage.find( '\r', pos );
   request.setVersion( rawMessage.substr( pos, versionEnd - pos ) );

   pos = versionEnd + 2;

   // Parse headers
   while ( pos < end )
   {
      size_t lineEnd = rawMessage.find( '\r', pos );
      if ( lineEnd == std::string::npos )
      {
         break;
      }

      size_t colon = rawMessage.find( ':', pos );
      if ( colon == std::string::npos )
      {
         break;
      }

      std::string key   = rawMessage.substr( pos, colon - pos );
      std::string value = rawMessage.substr( colon + 2, lineEnd - colon - 2 );

      std::transform( key.begin(), key.end(), key.begin(), ::tolower );
      std::transform( value.begin(), value.end(), value.begin(), ::tolower );

      request.setHeader( key, value );
      pos = lineEnd + 2;
   }

   return request;
}

};
