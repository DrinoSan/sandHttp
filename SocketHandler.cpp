// System Headers
#include <arpa/inet.h>
#include <sys/socket.h>

// Project Headers
#include "Connection.h"
#include "Exceptions.h"
#include "Log.h"
#include "SocketHandler.h"

namespace SandServer
{

//-----------------------------------------------------------------------------
SocketHandler_t::SocketHandler_t( int domain, int type, int protocol )
{
   //// PREPARE getaddrinfo structure
   // memset( &hints, 0, sizeof hints );
   //// From man pages
   //// When ai_family is set to PF_UNSPEC, it means the caller will accept any
   //// protocol family supported by the operating system.
   // hints.ai_family   = AF_UNSPEC;     // don't care IPv4 or IPv6
   // hints.ai_socktype = SOCK_STREAM;   // TCP stream sockets
   // hints.ai_flags    = AI_PASSIVE;    // fill in my IP for me

   socketFD = ::socket( domain, type, protocol );
   if ( socketFD == -1 )
   {
      throw std::runtime_error( "Failed to create socket: " +
                                std::string( std::strerror( errno ) ) );
   }
}

//-----------------------------------------------------------------------------
SocketHandler_t::~SocketHandler_t()
{
   if ( socketFD != -1 )
   {
      ::close( socketFD );
   }
}

//-----------------------------------------------------------------------------
void SocketHandler_t::init( const std::string& address, int port, int backlog )
{
   // Set socket options so that we can reuse the address quickly.
   int yes = 1;
   if ( setsockopt( socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) ==
        -1 )
   {
      throw std::runtime_error( "Failed to set socket options: " +
                                std::string( std::strerror( errno ) ) );
   }

   // Bind the socket.
   struct sockaddr_in addr;
   std::memset( &addr, 0, sizeof( addr ) );
   addr.sin_family = AF_INET;
   addr.sin_port   = htons( port );

   if ( address.empty() )
   {
      addr.sin_addr.s_addr = INADDR_ANY;
   }
   else
   {
      // inet_addr returns INADDR_NONE on error.
      addr.sin_addr.s_addr = inet_addr( address.c_str() );
      if ( addr.sin_addr.s_addr == INADDR_NONE )
      {
         throw std::runtime_error( "Invalid address provided to bind: " +
                                   address );
      }
   }

   if ( ::bind( socketFD, ( struct sockaddr* ) &addr, sizeof( addr ) ) == -1 )
   {
      throw std::runtime_error( "Failed to bind socket: " +
                                std::string( std::strerror( errno ) ) );
   }

   // Start listening on the socket.
   if ( ::listen( socketFD, backlog ) == -1 )
   {
      throw std::runtime_error( "Failed to listen on socket: " +
                                std::string( std::strerror( errno ) ) );
   }

   SLOG_INFO( "Socket bound and listening on port {0}", port );
}

//-----------------------------------------------------------------------------
int32_t SocketHandler_t::acceptConnection()
{
   struct sockaddr_in clientAddr;
   socklen_t          clientLen = sizeof( clientAddr );
   int                clientSocket =
       ::accept( socketFD, ( struct sockaddr* ) &clientAddr, &clientLen );
   if ( clientSocket == -1 )
   {
      SLOG_ERROR( "Failed to accept connection: {0}", std::strerror( errno ) );
   }

   return clientSocket;
}

//-----------------------------------------------------------------------------
void SocketHandler_t::closeSocket()
{
   if ( socketFD != -1 )
   {
      ::close( socketFD );
      socketFD = -1;
   }
}

//-----------------------------------------------------------------------------
int32_t SocketHandler_t::readFromSocket( Connection_t& conn )
{
   struct timeval timeout;
   timeout.tv_sec  = 10;
   timeout.tv_usec = 0;

   setsockopt( conn.socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof( timeout ) );

   char    recvBuffer[ BUFFER_SIZE ] = { 0 };
   int32_t bytesRead = recv( conn.socketFD, recvBuffer, BUFFER_SIZE, 0 );

   if ( bytesRead > 0 )
   {
      conn.persistentBuffer.append( recvBuffer, bytesRead );
   }
   else if ( bytesRead == 0 )
   {
      SLOG_WARN( "Client closed the connection" );
      throw ClientClosedConnectionException( "Client closed the connection" );
   }
   else if ( bytesRead < 0 )
   {
      if ( errno == EAGAIN || errno == EWOULDBLOCK )
      {
         SLOG_WARN( "Socket read timed out" );
         throw TimeoutException( "Socket read timed out." );
      }

      throw ServerExceptionIDontKnowWhatIamButIInheritFromRuntimeError(
          "Error receiving data from socket." );
   }

   return bytesRead;
}

//-----------------------------------------------------------------------------
void SocketHandler_t::writeToSocket( Connection_t&      conn,
                                     const std::string& data )
{
   int32_t bytesSent = 0;
   int32_t chunkSize = 0;
   size_t  dataSize  = data.size();

   while ( bytesSent < dataSize )
   {
      chunkSize =
          std::min( CHUNK_SIZE, static_cast<int32_t>( dataSize - bytesSent ) );

      SLOG_INFO( "Chunk size set to {0}", chunkSize );

      int32_t ret =
          send( conn.socketFD, data.c_str() + bytesSent, chunkSize, 0 );

      if ( ret == -1 )
      {
         SLOG_ERROR( "Connection reset by peer on socket {0} --- ERRNO: {1}",
                     conn.socketFD, errno );
         return;
      }

      bytesSent += ret;
   }
}

//-----------------------------------------------------------------------------
bool SocketHandler_t::hasIncomingData( int socketFD )
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

};   // namespace SandServer
