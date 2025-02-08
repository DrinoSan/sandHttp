// System Headers
#include <string.h>

// Project Headers
#include "SocketIOHandler.h"
#include "SocketHandler.h"
#include "HttpParser.h"

namespace SandServer
{
//-----------------------------------------------------------------------------
HTTPRequest_t SocketIOHandler_t::readHTTPMessage( Connection_t& conn )
{
   while ( conn.persistentBuffer.find( "\r\n\r\n" ) == std::string::npos )
   {
      SocketHandler_t::readFromSocket( conn );
   }

   size_t endOfRequestPos = conn.persistentBuffer.find( "\r\n\r\n" ) + 4;

   std::string completeRequest =
       conn.persistentBuffer.substr( 0, endOfRequestPos );

   conn.persistentBuffer.erase( 0, endOfRequestPos );

   return HttpParser_t::parseRequest( completeRequest );
}

//-----------------------------------------------------------------------------
void SocketIOHandler_t::writeHTTPMessage( Connection_t&         conn,
                                          const HTTPResponse_t& response )
{
   SocketHandler_t::writeToSocket( conn, response.getBody() );
}

//-----------------------------------------------------------------------------
bool SocketIOHandler_t::hasSocketDataToRead( int32_t socketFD )
{
   return SocketHandler_t::hasIncomingData( socketFD );
}
};   // namespace SandServer
