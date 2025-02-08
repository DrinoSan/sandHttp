// System Headers
#include <sys/socket.h>

// Project Headers
#include "Connection.h"
#include "Exceptions.h"
#include "Log.h"
#include "SocketHandler.h"

namespace SandServer
{
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
      throw ClientClosedConnectionException( "Client closed the connection" );
   }
   else if ( bytesRead < 0 )
   {
      if ( errno == EAGAIN || errno == EWOULDBLOCK )
      {
         throw TimeoutException( "Socket read timed out." );
      }

      throw std::runtime_error( "Error receiving data from socket." );
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
