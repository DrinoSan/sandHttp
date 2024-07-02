// Project HEADERS
#include "Log.h"
#include "Server.h"

int main( void )
{
    SandServer::Log_t::init();   // TODO move to Server_t constructor
    SLOG_INFO( "SANDRINO Initialized Log!" );

    // asio::io_service ios;
    SandServer::Server_t server;

    // TODO: It is not really necessary to pass response as a parameter... I
    // dont see the need for it right now!
    server.addRoute( "/home", SandServer::SAND_METHOD::GET,
                     []( SandServer::HTTPRequest_t&  request,
                         SandServer::HTTPResponse_t& response )
                     {
                         SLOG_INFO( "Called home" );
                         response.setBody( "<h1> Home </h1>" );
                         response.setHeader("content-type", "text/html");
                     } );

    server.serveStaticFiles("html", "/static");

    server.start( 8000 );

    return 0;
}
