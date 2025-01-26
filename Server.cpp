// System HEADERS
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <sys/event.h>
#include <sys/select.h>
#include <unistd.h>
#include <utility>
#include <vector>

// Project HEADERS
#include "HttpMessage.h"
#include "Log.h"
#include "Router.h"
#include "SandMethod.h"
#include "Server.h"
#include "SocketIOHandler.h"
#include "Exceptions.h"

namespace SandServer
{
// Defined in utils.cpp
std::vector<std::string> splitString( const std::string& str, char delimiter );
};   // namespace SandServer

// Not sure if this is a good strategy
volatile sig_atomic_t gotSigInt = 1;
int32_t               SOCKET_g;

//-----------------------------------------------------------------------------
void sigIntHandler( int sig )
{
   gotSigInt = 0;
   SLOG_WARN( "Got sigInt bye bye" );

   shutdown( SOCKET_g, SHUT_RDWR );
   close( SOCKET_g );
}

// General Helper functions
namespace SandServer
{

//-----------------------------------------------------------------------------
// get sockaddr, IPv4 or IPv6:
void* get_in_addr( struct sockaddr* sa )
{
   if ( sa->sa_family == AF_INET )
   {
      return &( ( ( struct sockaddr_in* ) sa )->sin_addr );
   }

   return &( ( ( struct sockaddr_in6* ) sa )->sin6_addr );
}

//-----------------------------------------------------------------------------
void printFilledGetAddrInfo( struct addrinfo* servinfo )
{
   struct addrinfo* p;
   char             ipstr[ INET6_ADDRSTRLEN ];

   for ( p = servinfo; p != nullptr; p = p->ai_next )
   {
      void*       addr;
      std::string ipver;
      if ( p->ai_family == AF_INET )
      {
         // IPv4
         auto* ipv4 = ( struct sockaddr_in* ) p->ai_addr;
         addr       = &( ipv4->sin_addr );
         ipver      = "IPv4";
      }
      else
      {
         // IPv6
         auto* ipv6 = ( struct sockaddr_in6* ) p->ai_addr;
         addr       = &( ipv6->sin6_addr );
         ipver      = "IPv6";
      }

      inet_ntop( p->ai_family, addr, ipstr, sizeof ipstr );
      SLOG_INFO( " {0}: {1}", ipver.c_str(), ipstr );
   }
}

//-----------------------------------------------------------------------------
void sigchld_handler( int s )
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while ( waitpid( -1, nullptr, WNOHANG ) > 0 )
      ;

   errno = saved_errno;
}
};   // namespace SandServer

// Main implementations
namespace SandServer
{

//-----------------------------------------------------------------------------
Server_t::Server_t() : Server_t( "config/config.toml" ) {}

//-----------------------------------------------------------------------------
Server_t::Server_t( std::string configPath )
    : config{ configPath }
{
	config.parse();
	config.dump();

   threadPool.init( config.num_workers );

   // PREPARE getaddrinfo structure
   memset( &hints, 0, sizeof hints );
   // From man pages
   // When ai_family is set to PF_UNSPEC, it means the caller will accept any
   // protocol family supported by the operating system.
   hints.ai_family   = AF_UNSPEC;     // don't care IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM;   // TCP stream sockets
   hints.ai_flags    = AI_PASSIVE;    // fill in my IP for me

   // Creating callback handler for child proceses
   // Reap dead childs... poor childs :(
   sa.sa_handler = sigchld_handler;
   sigemptyset( &sa.sa_mask );
   sa.sa_flags = SA_RESTART;
   if ( sigaction( SIGCHLD, &sa, NULL ) == -1 )
   {
      perror( "sigaction" );
      exit( 1 );
   }

   saInter.sa_handler = sigIntHandler;
   saInter.sa_flags   = 0;   // or SA_RESTART
   saInter.sa_mask    = 0;

   if ( sigaction( SIGINT, &saInter, nullptr ) == -1 )
   {
      SLOG_ERROR( "IN WHILE LOOP GOT SIGINT TRUE" );
   }
}

//-----------------------------------------------------------------------------
Server_t::~Server_t()
{
   threadPool.stop = true;
}

//-----------------------------------------------------------------------------
void Server_t::serveStaticFiles( std::string_view filePath,
                                 std::string_view urlPrefix )
{
   // We need to know the endpoint where static files are served
   if ( !urlPrefix.empty() && urlPrefix.front() == '/' )
   {
      urlPrefix.remove_prefix( 1 );
   }

   router.staticFilesUrlPrefix = urlPrefix;

   auto handler = [ this, filePath ]( const HTTPRequest_t& request,
                                      HTTPResponse_t&      response )
   {
      response = serveFile( filePath, request.getUrlParts() );
   };   // TODO: filePath -> dir or servingDir

   router.staticFileHandler = handler;

   // Serve files
   /// Content type based of file extension
   ///
}

//-----------------------------------------------------------------------------
HTTPResponse_t Server_t::serveFile( const fs::path&                 servingDir,
                                    const std::vector<std::string>& urlParts )
{
   // Get filepath
   // Append the requested path to the root directory
   HTTPResponse_t response;
   if ( urlParts.size() < 2 )
   {
      // No file provided
      response.notFound();
      return response;
   }

   std::string filePathPart;
   fs::path    filePath = servingDir;
   for ( std::string_view part : urlParts )
   {
      if ( part.front() == '/' )
      {
         part.remove_prefix( 1 );
      }

      // TODO: This check is shit
      if ( part == "static" )
      {
         continue;
      }

      filePath = filePath / fs::path( part );
   }

   // Check if the file exists
   if ( !fs::exists( filePath ) || fs::is_directory( filePath ) )
   {
      if ( fs::is_directory( filePath ) )
      {
         // TODO: Check if this really is logged if we access a folder only
         SLOG_WARN( "Requested path is a directory. We are returning 404" );
      }

      response.notFound();
      return response;
   }

   // Open the file in read-only mode
   std::ifstream fileFD{ filePath, std::ios::binary };

   if ( fileFD.is_open() )
   {
      std::string line;
      int32_t     idx = 1;
      while ( std::getline( fileFD, line ) )
      {
         // TODO: Create a function to only append body or create a
         // temprorary string variable and only call set body once
         response.setBody( response.getBody() + line );
      }
   }

   // Get the file size
   auto size = fs::file_size( filePath );

   // Set the Content-Type header based on the file's extension
   std::string content_type = "text/plain";
   if ( filePath.extension() == ".html" )
   {
      content_type = "text/html";
   }
   else if ( filePath.extension() == ".css" )
   {
      content_type = "text/css";
   }
   else if ( filePath.extension() == ".js" )
   {
      content_type = "application/javascript";
   }
   else if ( filePath.extension() == ".wasm" )
   {
      content_type = "application/wasm";
   }

   // Send the response headers
   response.setHeader( "Content-Type", content_type );
   response.setHeader( "Content-Length", std::to_string( size ) );

   // RVO should kick in right??? right????
   // We need to read the file
   return response;
}

//-----------------------------------------------------------------------------
bool Server_t::start( int32_t port_ )
{
   std::string port = ( port_ == 0 ) ? config.port : std::to_string( port_ );
   SLOG_WARN( "Sarting server on port: {0}", port );

   int rv;
   if ( ( rv = getaddrinfo( "0.0.0.0", port.c_str(), &hints, &servinfo ) ) !=
        0 )
   {
      SLOG_ERROR( "getaddrinfo: {0}", gai_strerror( rv ) );
      return false;
   }

   printFilledGetAddrInfo( servinfo );

   // loop through all the results and bind to the first we can
   int              yes = 1;
   struct addrinfo* p;
   for ( p = servinfo; p != nullptr; p = p->ai_next )
   {
      if ( ( socketFd = socket( p->ai_family, p->ai_socktype,
                                p->ai_protocol ) ) == -1 )
      {
         SLOG_ERROR( "Creation of socket failed" );
         continue;
      }

      // Setting socket as global variable for sigInt handler
      SOCKET_g = socketFd;

      // So I don't get the annoying failed to bind errors
      if ( setsockopt( socketFd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof( int ) ) == -1 )
      {
         SLOG_ERROR( "Setsockopt" );
         exit( 1 );
      }

      // bind it to the port we passed in to getaddrinfo():
      if ( bind( socketFd, p->ai_addr, p->ai_addrlen ) == -1 )
      {
         SLOG_ERROR( "Binding server with socketfd: {0} failed on port {1}",
                     socketFd, port );
         close( socketFd );
         continue;
      }

      break;
   }

   freeaddrinfo( servinfo );   // all done with this structure

   if ( p == nullptr )
   {
      SLOG_ERROR( "Was not able to fill addrinfo structure" );
      exit( 1 );
   }

   // Helloooo is there someone
   if ( listen( socketFd, config.back_log ) == -1 )
   {
      SLOG_ERROR( "Failed listen" );
      exit( 1 );
   }

   // start listnener thread here for incoming connections
   listenAndAccept();

   SLOG_INFO( "Leaving Sand Server" );

   return true;
}

//-----------------------------------------------------------------------------
void Server_t::listenAndAccept()
{
   char    ipStr[ INET6_ADDRSTRLEN ];   // Enough space to hold ipv4 or ipv6

   while ( true )
   {
      struct sockaddr_storage their_addr;   // connector's address information
      socklen_t               sin_size = 0;

      int newSocketFD = 0;

      socklen_t addr_size = sizeof their_addr;
      // accept is blocking so everything is cool
      newSocketFD =
          accept( socketFd, ( struct sockaddr* ) &their_addr, &addr_size );

      if ( gotSigInt == 0 )
      {
         threadPool.stop = true;
         break;
      }

      if ( newSocketFD == -1 )
      {
         SLOG_ERROR( "Accepting incoming connection failed with errno: {0}",
                     errno );
         continue;
      }

      inet_ntop( their_addr.ss_family,
                 get_in_addr( ( struct sockaddr* ) &their_addr ), ipStr,
                 sizeof ipStr );

      SLOG_TRACE( "Server got new incoming connection from {0}", ipStr );

      threadPool.enqueue(
          std::bind( &Server_t::processWorkerEvents, this, newSocketFD ) );
   }
}

//-----------------------------------------------------------------------------
std::pair<HTTPResponse_t, bool>
Server_t::generateResponse( HTTPRequest_t& httpRequest )
{
   httpRequest.printObject();
   SLOG_INFO( "\n\n------ END ------\n\n" );

   auto handler =
       router.matchRoute( httpRequest )
           .value_or( []( const HTTPRequest_t& req, HTTPResponse_t& resp )
                      { return resp.notFound(); } );

   // TODO: Here we create the session cookie and set header
   // SET-COOKIE
   // TODO: Make sure getHeader searches case insensitive.. or
   // convert all headers to lowercase and work with that
   auto cookie = httpRequest.getHeader( "cookie" ).value_or( "NO COOKIE" );

   // Checking for keep-alive
   bool keepAlive{ true };
   if ( httpRequest.getHeader( "connection" ).value_or( "" ) == "close" ||
        httpRequest.version == "HTTP/1.0" )
   {
      keepAlive = false;
   }

   HTTPResponse_t response;
   if ( keepAlive == true )
   {
      SLOG_WARN( " SETTING keep alive header" );
      response.setHeader( "connection", "keep-alive" );
   }

   // This feels ugly
   handler( httpRequest, response );
   response.prepareResponse();   // This is critical to call
                                 // because it sets content length

   return std::make_pair( response, keepAlive );
}

//-----------------------------------------------------------------------------
bool Server_t::hasIncomingData( int socketFD )
{
   fd_set         readfds;
   struct timeval tv;

   // Zero everything and then adding our FD we want to monitor
   FD_ZERO( &readfds );
   FD_SET( socketFD, &readfds );

   // 0 to be non-blocking
   tv.tv_sec  = 0;
   tv.tv_usec = 0;

   int32_t result = select( socketFD + 1, &readfds, NULL, NULL, &tv );

   if ( result < 0 )
   {
      SLOG_ERROR( "Error in select() for checking incoming data: {0}",
                  strerror( errno ) );
      return false;
   }

   return result > 0 && FD_ISSET( socketFD, &readfds );
}

//-----------------------------------------------------------------------------
bool Server_t::timeoutElapsed( const Connection& conn )
{
   // TODO make timeout setable via config
   const std::chrono::seconds IDLE_TIMEOUT( 10 );

   // Check if the connection has been idle longer than the timeout
   auto now          = std::chrono::steady_clock::now();
   auto idleDuration = now - conn.lastActivityTime;

   return idleDuration > IDLE_TIMEOUT;
}

//-----------------------------------------------------------------------------
void Server_t::processWorkerEvents( int32_t newSocketFD )
{
   Connection conn{ newSocketFD };
   conn.state = ConnectionState::IDLE;

   SLOG_INFO( "\n\n------ BEGIN: Got a message on the socket to read "
              "------\n\n" );
   // nodiscard will remind me to use the return value
   while ( conn.state != ConnectionState::CLOSED )
   {
      if ( conn.state == ConnectionState::IDLE )
      {
         if ( timeoutElapsed( conn ) )
         {
            SLOG_INFO( "Connection timeout for socket {0}", conn.socketFD );
            conn.state = ConnectionState::CLOSED;
            break;
         }

         if ( hasIncomingData( conn.socketFD ) )
         {
            conn.state = ConnectionState::ACTIVE;
            // Here we update the last time we got some fresh data
            conn.lastActivityTime = std::chrono::steady_clock::now();
         }
         else
         {
            // We did not get any data but due to keep-alive header we need to
            // wait until the timeout hits
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            continue;
         }
      }

      HTTPRequest_t  httpRequest;
      HTTPResponse_t httpResponse;
      bool           keepAlive;
      try
      {
         httpRequest =
             SandServer::SocketIOHandler_t::readHTTPMessage( conn.socketFD );

         std::pair<HTTPResponse_t, bool> result =
             generateResponse( httpRequest );

         httpResponse = result.first;
         keepAlive    = result.second;
      }
      catch ( const TimeoutException& ex )
      {
         SLOG_ERROR( "Timeout: {0} closing socket {1}", ex.what(),
                     newSocketFD );
         close( newSocketFD );
         return;
      }
      catch ( const ClientClosedConnectionException& ex )
      {
         SLOG_ERROR( "Client sent: 0 bytes. Closing socket {0}", newSocketFD );
         close( newSocketFD );
         return;
      }

      // Sending response
      SocketIOHandler_t::writeHTTPMessage( conn, httpResponse );

      if ( conn.state == ConnectionState::CLOSED )
      {
         SLOG_ERROR( "Connection is closed" );
         close( newSocketFD );
         return;
      }

      // TODO: make all headers either lowercase or uppercase look at this crap
      if ( keepAlive == false )
      {
         SLOG_ERROR( "CLOSING SOCKET HTTP 1/0 or connection == close" );
         close( newSocketFD );
         return;
      }
      else
      {
         conn.state = ConnectionState::IDLE;
      }
   }

   if ( conn.state == ConnectionState::CLOSED )
   {
      SLOG_ERROR( "Connection is closed" );
      close( newSocketFD );
      return;
   }
}

//-----------------------------------------------------------------------------
bool Server_t::addRoute( std::string&& route, const SAND_METHOD& method,
                         HandlerFunc handler )
{
   router.addRoute( route, std::move( handler ), method );
   return true;
}

}   // namespace SandServer

