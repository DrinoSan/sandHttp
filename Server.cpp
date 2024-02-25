// System HEADERS
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/event.h>
#include <unistd.h>

#include "Server.h"

// General Helper functions
namespace SandServer
{
//-----------------------------------------------------------------------------
void printFilledGetAddrInfo( struct addrinfo*  servinfo )
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

   // TODO: FINISDH kqueue event handling
   // Preparing kqueue worker threads
   for ( int i = 0; i < NUM_WORKERS; ++i )
   {
      if ( ( workerKqueueFD[ i ] = kqueue() ) < 0 )
      {
         fprintf( stderr, "Could not create worker fd for kqueue\n" );
         exit( EXIT_FAILURE );
      }
   }
}

//-----------------------------------------------------------------------------
bool Server_t::start( const std::string& ipAdr, int32_t port )
{
   int rv;
   if( ( rv = getaddrinfo( nullptr, std::to_string( port ).c_str(), &hints, &servinfo ) ) != 0 )
   {
      fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );
      return 1;
   }

   // loop through all the results and bind to the first we can
   int yes = 1;
   struct addrinfo* p;
   for ( p = servinfo; p != nullptr; p = p->ai_next )
   {
      if ( ( socketFd = socket( p->ai_family, p->ai_socktype,
                                p->ai_protocol ) ) == -1 )
      {
         printf( "server: socket\n" );
         continue;
      }

      // So I don't get the annoying failed to bind errors
      if ( setsockopt( socketFd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof( int ) ) == -1 )
      {
         printf( "setsockopt\n" );
         exit( 1 );
      }

      // bind it to the port we passed in to getaddrinfo():
      if ( bind( socketFd, p->ai_addr, p->ai_addrlen ) == -1 )
      {
         close( socketFd );
         printf( "server: bind\n" );
         continue;
      }

      break;
   }

   freeaddrinfo( servinfo );   // all done with this structure

   if ( p == nullptr )
   {
      fprintf( stderr, "server: failed to bind\n" );
      exit( 1 );
   }

   // Helloooo is there someone
   if ( listen( socketFd, BACK_LOG ) == -1 )
   {
      fprintf( stderr, "listen\n" );
      exit( 1 );
   }

   return true;
}

}
