// System Headers

// Project Headers
#include "HttpParser.h"
#include "Exceptions.h"
#include "Utils.h"

namespace SandServer
{
static StringContainer_t supportedHTTPMethods{ "GET", "POST" };
bool                     isSupportedHTTPMethod( std::string& method )
{
   return has( supportedHTTPMethods, method );
}

//-----------------------------------------------------------------------------
HTTPRequest_t HttpParser_t::parseRequest( const std::string& rawMessage )
{
   HTTPRequest_t request;
   size_t        pos = 0;
   size_t        end = rawMessage.find( "\r\n\r\n" );

   if ( end == std::string::npos )
   {
      throw ParsingExceptionMissingHeaderDelimiter_t(
          "Invalid HTTP request: Missing header delimiter (CRLF CRLF)." );
   }

   auto lineEnd = rawMessage.find( "\r\n" );
   if ( lineEnd == std::string::npos )
   {
      throw ParsingExceptionMissingRequestline_t(
          "Invalid HTTP request: No request line found" );
   }

   // Getting the status line
   std::string statusLine = rawMessage.substr( pos, lineEnd - pos );
   auto        tokens     = Utils::splitString( statusLine, ' ' );

   if ( tokens.size() != 3 )
   {
      throw ParsingExceptionMalformedStatusLine_t(
          "Invalid HTTP request: Malformed status line" );
   }

   // Checking for HTTP method
   auto method = tokens[ 0 ];
   if ( method.empty() )
   {
      throw ParsingExceptionUnsupportedHTTPMethod_t(
          "Invalid HTTP request: HTTP method is empty." );
   }

   if ( !isSupportedHTTPMethod( method ) )
   {
      throw ParsingExceptionUnsupportedHTTPMethod_t(
          "Invalid HTTP request: HTTP method is not supported WIP" );
   }

   request.setMethod( method );

   // Checking the URI
   std::string uri = tokens[ 1 ];
   if ( uri.empty() )
   {
      throw ParsingExceptionMalformedStatusLine_t(
          "Invalid HTTP request: URI not set" );
   }

   request.setURI( uri );

   // Checking  the HTTP version (should be HTTP/1.1 or HTTP/2)
   std::string version = tokens[ 2 ];
   if ( version.compare( 0, 5, "HTTP/" ) != 0 )
   {
      throw ParsingExceptionMalformedStatusLine_t(
          "Invalid HTTP request: HTTP version is malformed." );
   }

   request.setVersion( version );

   // Moving pos after \r\n
   pos = lineEnd + 2;

   // Parse headers
   while ( pos < end )
   {
      // Finding the end of the current header line
      size_t headerEnd = rawMessage.find( "\r\n", pos );
      if ( headerEnd == std::string::npos )
      {
         // I think this can not happen. Because we checked already for the end
         // delimiter
         break;
      }

      // If we have an empty header line we break the fuck out
      if ( headerEnd == pos )
      {
         pos += 2;   // Skip the CRLF
         break;
      }

      size_t colon = rawMessage.find( ':', pos );
      // If colon pos is > than the current line headerEnd we have a problem
      if ( colon == std::string::npos || colon > headerEnd )
      {
         throw ParsingExceptionMalformedHeader_t(
             "Invalid HTTP request: Malformed header (missing colon)" );
      }

      std::string key   = rawMessage.substr( pos, colon - pos );
      std::string value = rawMessage.substr( colon + 1, headerEnd - colon - 1 );

      // Trim leading spaces from value
      size_t valueStart = value.find_first_not_of( " \t" );
      if ( valueStart != std::string::npos )
      {
         value = value.substr( valueStart );
      }

      if ( key.empty() || value.empty() )
      {
         throw ParsingExceptionMalformedHeader_t(
             "Invalid HTTP request: Header key or value is empty." );
      }

      std::transform( key.begin(), key.end(), key.begin(), ::tolower );
      std::transform( value.begin(), value.end(), value.begin(),
                      ::tolower );   // TODO: Am i allowed to set lowercase all
                                     // the values from the client??????

      request.setHeader( key, value );
      pos = headerEnd + 2;   // Move me baby to the next line
   }

   // Checking for mandatory header value
   if ( request.getHeader( "host" ).value_or( "" ).empty() )
   {
      throw ParsingExceptionMalformedHeader_t(
          "Invalid HTTP request: Missing Host header." );
   }

   // Body parsing
   if ( pos < rawMessage.size() )
   {
      if ( !request.getHeader( "content-length" ).value_or( "" ).empty() )
      {
         size_t contentLength =
             std::stoul( request.getHeader( "content-length" ).value_or( "" ) );

         if ( rawMessage.size() < pos + contentLength )
         {
            throw ParsingExceptionMalformedHeader_t(
                "Incomplete HTTP request body." );
         }

         std::string body = rawMessage.substr( pos, contentLength );
         request.setBody( body );
      }
      else if ( request.getHeader( "transfer-encoding" ).value_or( "" ) ==
                "chunked" )
      {
         std::string body;
         while ( true )
         {
            // Read the chunk size line.
            size_t chunkSizeEnd = rawMessage.find( "\r\n", pos );
            if ( chunkSizeEnd == std::string::npos )
            {
               throw ParsingExceptionMalformedHeader_t(
                   "Malformed chunked encoding: Missing chunk size line." );
            }

            std::string chunkSizeStr =
                rawMessage.substr( pos, chunkSizeEnd - pos );

            // Trim whitespace from chunkSizeStr
            chunkSizeStr.erase( 0, chunkSizeStr.find_first_not_of( " \t" ) );
            chunkSizeStr.erase( chunkSizeStr.find_last_not_of( " \t" ) + 1 );

            // Debug log the chunk size string
        SLOG_INFO("Parsing chunk size: \"{0}\"", chunkSizeStr);

            // TODO: Check if this works...
            size_t chunkSize = std::stoul( chunkSizeStr, nullptr,
                                           16 );   // Hexadecimal conversion.
            pos              = chunkSizeEnd + 2;   // Skip the chunk size line.

            if ( chunkSize == 0 )
            {
               // Last chunk; skip the trailing CRLF.
               pos += 2;
               break;
            }

            if ( rawMessage.size() < pos + chunkSize )
            {
               throw ParsingExceptionMalformedHeader_t(
                   "Incomplete chunked body." );
            }

            body.append( rawMessage.substr( pos, chunkSize ) );
            pos += chunkSize;

            // Each chunk is terminated by CRLF.
            if ( rawMessage.substr( pos, 2 ) != "\r\n" )
            {
               throw ParsingExceptionMalformedHeader_t(
                   "Malformed chunked encoding: Missing CRLF after chunk "
                   "data." );
            }

            pos += 2;
         }

         request.setBody( body );
      }
      else
      {
         // No Content-Length or chunked encoding; assume no body.
         // Empty body
         request.setBody( "" );
      }
   }
   else
   {
      // Empty body
      request.setBody( "" );
   }

   return request;
}

};   // namespace SandServer
