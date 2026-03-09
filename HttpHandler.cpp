// System Headers
#include <unistd.h>

// Project Headers
#include "Exceptions.h"
#include "HttpHandler.h"
#include "HttpMessage.h"
#include "Log.h"
#include "SocketIOHandler.h"

namespace SandServer
{
//-----------------------------------------------------------------------------
static bool shouldKeepAlive( const HTTPRequest_t& request )
{
   bool keepAlive  = request.version == "HTTP/1.1";
   auto connHeader = request.getHeader( "connection" ).value_or( "" );
   if ( connHeader == "close" )
      return false;
   if ( connHeader == "keep-alive" )
      return true;
   return keepAlive;
}

//-----------------------------------------------------------------------------
HandlerFunc getHandler( Router_t& router, HTTPRequest_t& httpRequest )
{
   httpRequest.printObject();
   SLOG_INFO( "\n\n------ END ------\n\n" );

   auto handler =
       router.matchRoute( httpRequest )
           .value_or( []( const HTTPRequest_t& req, HTTPResponse_t& resp )
                      { return resp.notFound(); } );

   return handler;
}

//-----------------------------------------------------------------------------
void HttpHandler_t::handleConnection( Connection_t& conn, Router_t& router )
{
   // Create a Connection_t object, initialize a persistent buffer, etc.
   SLOG_INFO( "HTTP connection accepted on socket {0}", conn.socketFD );

   // TODO: currently we dont really use ConnectionState_t and
   // lastActivityTimeout...
   while ( conn.state != ConnectionState_t::CLOSED )
   {
      try
      {
         HTTPRequest_t request = socketIOHandler.readHTTPMessage( conn );
         conn.lastActivityTime = std::chrono::steady_clock::now();

         auto handler   = getHandler( router, request );
         auto keepAlive = shouldKeepAlive( request );

         HTTPResponse_t response;
         response.setHeader( "connection", keepAlive ? "keep-alive" : "close" );
         handler( request, response );

         socketIOHandler.writeHTTPMessage( conn, response.serialize() );

         if ( !keepAlive )
         {
            SLOG_INFO( "Keep-alive not requested; closing socket {0}",
                       conn.socketFD );
            break;
         }
      }
      catch ( const TimeoutException& ex )
      {
         SLOG_ERROR( "Timeout occurred: {0}. Closing socket {1}", ex.what(),
                     conn.socketFD );
         break;
      }
      catch ( const ClientClosedConnectionException& ex )
      {
         SLOG_ERROR( "Client closed connection on socket {0}", conn.socketFD );
         break;
      }
      catch ( const ParsingException_t& ex )
      {
         SLOG_ERROR( "{0}", ex.what() );
         break;
      }
      catch ( UnexpectedSocketException& ex )
      {
         SLOG_ERROR( "I have no idea what just happend {0}", conn.socketFD );
         assert( false );
      }
      catch ( const std::exception& ex )
      {
         SLOG_ERROR( "Unexpected error on socket {0}: {1}", conn.socketFD,
                     ex.what() );
         break;
      }
   }

   close( conn.socketFD );
   SLOG_INFO( "Socket {0} closed", conn.socketFD );
}

//-----------------------------------------------------------------------------
// HTTPResponse_t HttpHandler_t::generateResponse( HandlerFunc&   handler,
//                                                HTTPRequest_t& httpRequest )
//{
//   // TODO: Here we create the session cookie and set header
//   // SET-COOKIE
//   auto cookie = httpRequest.getHeader( "cookie" ).value_or( "NO COOKIE" );
//
//   // Checking for keep-alive
//   // This feels ugly
//   //handler( httpRequest, response );
//
//   //return response;
//}

};   // namespace SandServer
