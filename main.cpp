// Project HEADERS
#include "Log.h"
#include "Server.h"

int main( void )
{
    SandServer::Log_t::init();   // TODO move to Server_t constructor
    SLOG_INFO( "SANDRINO Initialized Log!" );

    // asio::io_service ios;
    SandServer::Server_t server;

    server.addRoute( "/home", SandServer::SAND_METHOD::GET,
                     []( SandServer::HTTPRequest_t&  request,
                         SandServer::HTTPResponse_t& response )
                     {
                         SLOG_INFO( "Called home" );
                         response.setBody( "<h1> Home </h1>" );
                         response.setHeader( "content-type", "text/html" );
                     } );

    server.addRoute( "/pathvalue/{pathname}", SandServer::SAND_METHOD::GET,
                     []( SandServer::HTTPRequest_t&  request,
                         SandServer::HTTPResponse_t& response )
                     {
                         SLOG_INFO( "Called pathvalue" );
                         std::string pathName = request.pathValue( "pathname" );
                         std::string pathValue{ "<h1> Requested pathname/" };
                         pathValue = pathValue + pathName + " </h1>";
                         response.setBody( pathValue );
                         response.setHeader( "content-type", "text/html" );
                     } );

    server.addRoute(
        "/session", SandServer::SAND_METHOD::GET,
        [ &server ]( SandServer::HTTPRequest_t&  request,
                     SandServer::HTTPResponse_t& response )
        {
            SLOG_INFO( "Called home" );
            response.setBody( "<h1> Trying to set session cookie </h1>" );
            response.setHeader( "content-type", "text/html" );
            // TODO COOKIE WIP
            response.setHeader( "set-cookie",
                                "sessionID=123123123 expires=Date: Thu, 11 Jul "
                                "2024 07:28:00 GMT" );
        } );

    server.serveStaticFiles( "html", "/static" );

    server.start( 8000 );

    return 0;
}
