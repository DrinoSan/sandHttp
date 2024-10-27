// System HEADERS
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <sys/_types/_size_t.h>
#include <sys/event.h>
#include <unistd.h>
#include <utility>
#include <vector>

// Project HEADERS
#include "Exceptions.h"
#include "HttpMessage.h"
#include "Log.h"
#include "Router.h"
#include "SandMethod.h"
#include "Server.h"
#include "SocketIOHandler.h"

namespace SandServer
{
// Defined in utils.cpp
std::vector<std::string> splitString( const std::string& str, char delimiter );
};   // namespace SandServer

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
   struct addrinfo* p = nullptr;
   char             ipstr[ INET6_ADDRSTRLEN ];

   for ( p = servinfo; p != nullptr; p = p->ai_next )
   {
      void*       addr = nullptr;
      std::string ipver;
      if ( p->ai_family == AF_INET )
      {
         // IPv4
         auto* ipv4 = reinterpret_cast<struct sockaddr_in*>( p->ai_addr );
         addr       = &( ipv4->sin_addr );
         ipver      = "IPv4";
      }
      else
      {
         // IPv6
         auto* ipv6 = reinterpret_cast<struct sockaddr_in6*>( p->ai_addr );
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
Server_t::Server_t( std::string configPath ) : config{ configPath }
{
   config.parse();
   config.dump();
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

   // Preparing kqueue worker threads
   for ( int& i : workerKqueueFD )
   {
      if ( ( i = kqueue() ) < 0 )
      {
         fprintf( stderr, "Could not create worker fd for kqueue\n" );
         exit( EXIT_FAILURE );
      }
   }

   // Seting array to sane values
   for ( int i = 0; i < config.num_workers; ++i )
   {
      memset( &wrkEvents[ i ], 0, sizeof wrkEvents[ i ] );
      memset( &wrkChangedEvents[ i ], 0, sizeof wrkChangedEvents[ i ] );
   }

   if ( ( kq = kqueue() ) == -1 )
   {
      perror( "kqueue" );
      exit( EXIT_FAILURE );
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

   int rv = 0;
   if ( ( rv = getaddrinfo( config.host.c_str(), config.port.c_str(), &hints,
                            &servinfo ) ) != 0 )
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
      // TODO: Disgusting
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
   listenerThread = std::thread( &Server_t::listenAndAccept, this );

   for ( int32_t i = 0; i < config.num_workers; ++i )
   {
      workerThread[ i ] =
          std::thread( &Server_t::processWorkerEvents, this, i );
   }

   listenerThread.join();

   for ( auto& thread : workerThread )
   {
      thread.join();
   }

   SLOG_INFO( "Leaving Sand Server" );

   return true;
}

//-----------------------------------------------------------------------------
void Server_t::listenAndAccept()
{
   int32_t workerIdx{ 0 };
   std::array<char, INET6_ADDRSTRLEN>
       ipStr;   // Enough space to hold ipv4 or ipv6

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
         break;
      }

      if ( newSocketFD == -1 )
      {
         SLOG_ERROR( "Accepting incoming connection failed with errno: {0}",
                     errno );
         continue;
      }

      inet_ntop( their_addr.ss_family,
                 get_in_addr( ( struct sockaddr* ) &their_addr ), ipStr.data(),
                 ipStr.size() );
      // inet_ntop( their_addr.ss_family,
      //					  get_in_addr( ( struct
      //sockaddr* ) &their_addr ), ipStr, 					  sizeof ipStr );

      SLOG_TRACE( "Server got new incoming connection from {0}", ipStr.data() );

      // EV_SET(Something something);
      // Please check:
      // https://wiki.netbsd.org/tutorials/kqueue_tutorial/#:~:text=A%20kevent%20is%20identified%20by,to%20process%20the%20respective%20event.
      for ( int32_t i = 0; i < config.num_k_events; ++i )
      {
         // Checking for a slot which available (No socket assigned)
         if ( wrkChangedEvents[ workerIdx ][ i ].ident != 0 )
         {
            continue;
         }

         inet_ntop( their_addr.ss_family,
                    get_in_addr( ( struct sockaddr* ) &their_addr ),
                    ipStr.data(), ipStr.size() );

         SLOG_TRACE( "Server got new incoming connection from {0}",
                     ipStr.data() );
         // Here is the point where we have to add the Session object for
         // this connection then our EV_SET call would look like this:
         //
         /// EV_SET( &wrkChangedEvents[ workerIdx ][ i ], newSocketFD,
         ///         EVFILT_READ, EV_ADD, 0, 0, SESSION_OBJECT );

         EV_SET( &wrkChangedEvents[ workerIdx ][ i ], newSocketFD, EVFILT_READ,
                 EV_ADD, 0, 0, 0 );

         // We have also worker threads and each worker thread should have
         // its own event list kevent(Something something); I set 1 because
         // we only add one element to observation
         if ( kevent( workerKqueueFD[ workerIdx ],
                      &wrkChangedEvents[ workerIdx ][ i ], 1, nullptr, 0,
                      nullptr ) < 0 )
         {
            if ( errno != 0 )
            {
               exit( EXIT_FAILURE );
            }
         }

         break;
      }

      ++workerIdx;
      if ( workerIdx == config.num_workers )
      {
         workerIdx = 0;
      }
   }
}

//-----------------------------------------------------------------------------
void Server_t::processWorkerEvents( int32_t workerIdx )
{
   // Kqueu FD
   const int32_t workerKFd = workerKqueueFD[ workerIdx ];

   int32_t numEvents{ 0 };

   struct timespec timeout;
   timeout.tv_sec  = 1;   // Check the flag every second
   timeout.tv_nsec = 0;

   while ( true )
   {
      numEvents = kevent( workerKFd, nullptr, 0, wrkEvents[ workerIdx ],
                          config.num_k_events, &timeout );

      if ( gotSigInt == 0 )
      {
         break;
      }

      if ( numEvents == -1 )
      {
         // Huston we got a problem
         exit( EXIT_FAILURE );
      }

      for ( int32_t i = 0; i < numEvents; ++i )
      {
         auto& event = wrkEvents[ workerIdx ][ i ];

         if ( event.flags & EV_EOF )
         {
            // From what I understood EV_EOF is sent when nothing is to be
            // read from socket
            // TODO: Here we need to check
            // If Client really closed connection
            /// Header connection close
            /// When the session/connection does not have a keep alive
            /// Reached threshold of keep alive
            ///  To check keep alive i can get the session data from udate
            SLOG_INFO( "Client closed Connection" );

            struct kevent deleteEvent;
            EV_SET( &deleteEvent, event.ident, EVFILT_READ, EV_DELETE, 0, 0,
                    NULL );

            kevent( workerKFd, &deleteEvent, 1, nullptr, 0, nullptr );

            continue;
         }

         if ( event.flags & EVFILT_READ )
         {
            SLOG_INFO( "\n\n------ BEGIN: Got a message on the socket to read "
                       "------\n\n" );
            // nodiscard will remind me to use the return value
            auto httpMessage =
                SandServer::SocketIOHandler_t::readHTTPMessage( event.ident );

            httpMessage.printObject();
            SLOG_INFO( "\n\n------ END ------\n\n" );

            auto handler = router.matchRoute( httpMessage )
                               .value_or( []( const HTTPRequest_t& req,
                                              HTTPResponse_t&      resp )
                                          { return resp.notFound(); } );

            // TODO: Here we create the session cookie and set header
            // SET-COOKIE
            // TODO: Make sure getHeader searches case insensitive.. or
            // convert all headers to lowercase and work with that
            auto cookie =
                httpMessage.getHeader( "Cookie" ).value_or( "NO COOKIE" );

            // This feels ugly
            HTTPResponse_t response;
            handler( httpMessage, response );
            response.prepareResponse();   // This is critical to call
                                          // because it sets content length

            // Sending response
            SocketIOHandler_t::writeHTTPMessage( event.ident, response );
         }
      }
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
