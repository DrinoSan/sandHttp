// Project HEADERS
#include "Log.h"
#include "Server.h"
#include "asio.hpp"

int main()
{
   SandServer::Log_t::init();   // TODO move to Server_t constructor
   SLOG_INFO( "SANDRINO Initialized Log!" );

   asio::io_service     ios;
   SandServer::Server_t server;

   server.addRoute( "/home", SandServer::SAND_METHOD::GET,
                    []( const SandServer::HTTPRequest_t& request,
                        SandServer::HTTPResponse_t&      response )
                    {
                       SLOG_INFO( "Called home" );
                       response.setBody( "<h1> Home </h1>" );
                       response.setHeader( "content-type", "text/html" );
                    } );

   server.addRoute( "/pathvalue/{pathname}/foo/fiz",
                    SandServer::SAND_METHOD::GET,
                    []( const SandServer::HTTPRequest_t& request,
                        SandServer::HTTPResponse_t&      response )
                    {
                       SLOG_INFO( "Called pathvalue foo" );
                       std::string pathName = request.pathValue( "pathname" );
                       std::string pathValue{ "<h1> foo Requested pathname/" };
                       pathValue = pathValue + pathName + " </h1>";
                       response.setBody( pathValue );
                       response.setHeader( "content-type", "text/html" );
                    } );

   server.addRoute( "/pathvalue/{pathname}/bar/fiz",
                    SandServer::SAND_METHOD::GET,
                    []( const SandServer::HTTPRequest_t& request,
                        SandServer::HTTPResponse_t&      response )
                    {
                       SLOG_INFO( "Called pathvalue bar" );
                       std::string pathName = request.pathValue( "pathname" );
                       std::string pathValue{ "<h1> bar Requested pathname/" };
                       pathValue = pathValue + pathName + " </h1>";
                       response.setBody( pathValue );
                       response.setHeader( "content-type", "text/html" );
                    } );

   server.addRoute(
       "/session", SandServer::SAND_METHOD::GET,
       [ &server ]( const SandServer::HTTPRequest_t& request,
                    SandServer::HTTPResponse_t&      response )
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

   server.start();

   return 0;
}
