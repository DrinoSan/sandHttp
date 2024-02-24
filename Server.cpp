// System HEADERS
#include <arpa/inet.h>
#include <unistd.h>

// Project HEADERS
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
};

namespace SandServer
{

//-----------------------------------------------------------------------------
Server_t::Server_t()
{
   memset(&hints, 0, sizeof hints);
   // From man pages
   // When ai_family is set to PF_UNSPEC, it means the caller will accept any protocol
   // family supported by the operating system.
   hints.ai_family = AF_UNSPEC;        // don't care IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
   hints.ai_flags = AI_PASSIVE;        // fill in my IP for me
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
   for ( auto* p = servinfo; p != nullptr; p = p->ai_next )
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

   return true;
}

}
