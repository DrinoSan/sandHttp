// System Headers
#include <string>

// Project Headers
#include "HttpParser.h"
#include "SocketHandler.h"
#include "SocketIOHandler.h"
#include "Exceptions.h"


constexpr size_t MAX_HEADER_SIZE = 8192; // 8KB

namespace SandServer
{
//-----------------------------------------------------------------------------
HTTPRequest_t SocketIOHandler_t::readHTTPMessage( Connection_t& conn )
{
   while ( conn.persistentBuffer.find( "\r\n\r\n" ) == std::string::npos )
   {
      // No complete request in buffer yet, so read more data.
      // This call appends any new data into conn.persistentBuffer.
      SocketHandler_t::readFromSocket( conn );

      // Catching requests which would make me die
      // Example missing \r\n\r\n
      if( conn.persistentBuffer.size() > MAX_HEADER_SIZE )
      {
         throw RequestHeaderException( "Request header too large" );
      }
   }

   size_t endOfRequestPos = conn.persistentBuffer.find( "\r\n\r\n" ) + 4;

   std::string completeRequest =
       conn.persistentBuffer.substr( 0, endOfRequestPos );

   // I cant call clear beacuse it is possible that i have data read beyond
   // /r/n/r/n
   conn.persistentBuffer.erase( 0, endOfRequestPos );

   HTTPRequest_t request = HttpParser_t::parseRequest( completeRequest );

   // Trimming begining possible whitespaces
   size_t firstNonWhitespace =
       conn.persistentBuffer.find_first_not_of( " \t\r\n" );

   if ( firstNonWhitespace != std::string::npos )
   {
      conn.persistentBuffer.erase( 0, firstNonWhitespace );
   }

   return request;
}

//-----------------------------------------------------------------------------
void SocketIOHandler_t::writeHTTPMessage( Connection_t&         conn,
                                          const std::string& response )
{
   SocketHandler_t::writeToSocket( conn, response );
}

//-----------------------------------------------------------------------------
bool SocketIOHandler_t::hasSocketDataToRead( int32_t socketFD )
{
   return SocketHandler_t::hasIncomingData( socketFD );
}
};   // namespace SandServer
