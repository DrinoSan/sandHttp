// Project HEADERS
#include "Server.h"
#include "Log.h"

int main(void)
{
    SandServer::Log_t::init();   // TODO move to Server_t constructor
    SLOG_INFO( "SANDRINO Initialized Log!" );

    // asio::io_service ios;
    SandServer::Server_t server;

    server.addRoute( "/home", SandServer::SAND_METHOD::GET,
                     []( SandServer::HTTPRequest_t&  request,
                         SandServer::HTTPResponse_t& response )
                     { SLOG_INFO( "Called home" ); } );

    server.start( 8000 );

    return 0;
}
