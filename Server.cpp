// System HEADERS
#include <arpa/inet.h>
#include <errno.h>
#include <memory>
#include <signal.h>
#include <sys/event.h>
#include <unistd.h>
#include <vector>

// Project HEADERS
#include "Server.h"
#include "SocketIOHandler.h"
#include "Log.h"

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
   struct addrinfo *p;
   char ipstr[INET6_ADDRSTRLEN];

   for( p = servinfo; p != NULL; p = p->ai_next )
   { 
      void *addr;
      std::string ipver;
      if ( p->ai_family == AF_INET )
      { 
         // IPv4
         struct sockaddr_in *ipv4 = ( struct sockaddr_in * )p->ai_addr; addr = &( ipv4->sin_addr );
         ipver = "IPv4";
      }
      else
      {
         // IPv6
         struct sockaddr_in6 *ipv6 = ( struct sockaddr_in6 * )p->ai_addr; addr = &( ipv6->sin6_addr );
         ipver = "IPv6";
      }

      inet_ntop( p->ai_family, addr, ipstr, sizeof ipstr );
      printf( " %s: %s\n", ipver.c_str(), ipstr ); 
   }
}

//-----------------------------------------------------------------------------
void sigchld_handler( int s )
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while ( waitpid( -1, NULL, WNOHANG ) > 0 )
      ;
   errno = saved_errno;
}
};

// Main implementations
namespace SandServer
{

//-----------------------------------------------------------------------------
Server_t::Server_t()
{
   // PREPARE getaddrinfo structure
   memset(&hints, 0, sizeof hints);
   // From man pages
   // When ai_family is set to PF_UNSPEC, it means the caller will accept any protocol
   // family supported by the operating system.
   hints.ai_family = AF_UNSPEC;        // don't care IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
   hints.ai_flags = AI_PASSIVE;        // fill in my IP for me

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
   for ( int i = 0; i < NUM_WORKERS; ++i )
   {
      if ( ( workerKqueueFD[ i ] = kqueue() ) < 0 )
      {
         fprintf( stderr, "Could not create worker fd for kqueue\n" );
         exit( EXIT_FAILURE );
      }
   }

   // Seting array to sane values
   for ( int i = 0; i < NUM_WORKERS; ++i )
   {
      memset( &wrkEvents[ i ], 0, sizeof wrkEvents[ i ] );
      memset( &wrkChangedEvents[ i ], 0, sizeof wrkChangedEvents[ i ] );
   }

   if ( ( kq = kqueue() ) == -1 )
   {
      perror( "kqueue" );
      exit( EXIT_FAILURE );
   }
}

//-----------------------------------------------------------------------------
bool Server_t::start( int32_t port )
{
   SLOG_WARN( "Sarting server on port: {0}", port );

   int rv;
   if( ( rv = getaddrinfo( nullptr, std::to_string( port ).c_str(), &hints, &servinfo ) ) != 0 )
   {
      SLOG_ERROR( "getaddrinfo: {0}", gai_strerror( rv ) );
      return 1;
   }

   printFilledGetAddrInfo( servinfo );

   // loop through all the results and bind to the first we can
   int yes = 1;
   struct addrinfo* p;
   for ( p = servinfo; p != nullptr; p = p->ai_next )
   {
      if ( ( socketFd = socket( p->ai_family, p->ai_socktype,
                                p->ai_protocol ) ) == -1 )
      {
         SLOG_ERROR( "Creation of socket failed" );
         continue;
      }

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
   if ( listen( socketFd, BACK_LOG ) == -1 )
   {
      SLOG_ERROR( "Failed listen" );
      exit( 1 );
   }

   // start listnener thread here for incoming connections
   listenerThread = std::thread( &Server_t::listenAndAccept, this );

   for ( int32_t i = 0; i < NUM_WORKERS; ++i )
   {
      workerThread[ i ] =
          std::thread( &Server_t::processWorkerEvents, this, i );
   }

   listenerThread.join();

   for ( int32_t i = 0; i < NUM_WORKERS; ++i )
   {
      workerThread[ i ].join();
   }

   return true;
}

//-----------------------------------------------------------------------------
void Server_t::listenAndAccept()
{
   int32_t workerIdx{ 0 };
   char    ipStr[ INET6_ADDRSTRLEN ];   // Enough space to hold ipv4 or ipv6

   while ( true )
   {
      struct sockaddr_storage their_addr;   // connector's address information
      socklen_t               sin_size;

      int newSocketFD;

      socklen_t addr_size = sizeof their_addr;
      // accept is blocking so everything is cool
      newSocketFD =
          accept( socketFd, ( struct sockaddr* ) &their_addr, &addr_size );

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

      // EV_SET(Something something);
      // Please check:
      // https://wiki.netbsd.org/tutorials/kqueue_tutorial/#:~:text=A%20kevent%20is%20identified%20by,to%20process%20the%20respective%20event.
      for ( int32_t i = 0; i < NUM_K_EVENTS; ++i )
      {
         // Checking for a slot which available (No socket assigned)
         if ( wrkChangedEvents[ workerIdx ][ i ].ident != 0 )
         {
             continue;
         }

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
      if ( workerIdx == NUM_WORKERS )
      {
         workerIdx = 0;
      }
   }
}

//-----------------------------------------------------------------------------
void Server_t::processWorkerEvents( int32_t workerIdx )
{
   // Kqueu FD
   int32_t workerKFd = workerKqueueFD[ workerIdx ];

   int32_t numEvents;

   while ( true )
   {
      numEvents = kevent( workerKFd, nullptr, 0, wrkEvents[ workerIdx ],
                          NUM_K_EVENTS, nullptr );

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
             // From what I understood EV_EOF is sent when nothing is to be read
             // from socket
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
             kevent( workerKFd, &deleteEvent, 1, NULL, 0, NULL );
             // close( event.ident );
             continue;
         }

         if ( event.flags & EVFILT_READ )
         {
             SLOG_INFO( "Got a message on the socket to read" );
             // nodiscard will remind me to use the return value
             auto httpMessage =
                 SandServer::SocketIOHandler_t::readHTTPMessage( event.ident );
             // Great now we have stuff in the buffer but now we need to handle
             // it
             // So here comes probably routing into play?!
             // Here we create the session cookie and set header SET-COOKIE

             // Sending response
             SocketIOHandler_t::writeHTTPMessage( event.ident,
                                                  HTTPRequest_t() );
         }
      }
   }
}

}   // namespace SandServer
