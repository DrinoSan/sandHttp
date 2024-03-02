// System Headers
#include <vector>
#include <sys/socket.h>

// Project Headers
#include "SocketIOHandler.h"


namespace SandServer
{
   //-----------------------------------------------------------------------------
   Request_t SocketIOHandler_t::readHTTPMessage( int socketFD )
   {
      constexpr int32_t BUFFER_SIZE{ 1024 };
      char recvBuffer[BUFFER_SIZE]; 
      memset( recvBuffer, 0, BUFFER_SIZE );


      std::string httpMessageBuffer;
      int32_t     readBytes{ 0 };

      // When is a full message read
      // 1) We get a /r/n/r/n
      // 2) We read 0 bytes
      while( ( readBytes = recv( socketFD, recvBuffer, BUFFER_SIZE, 0 ) ) )
      {
         if( readBytes < 0 )
         {
            // We got a error
            break;
         }

         // C++ style to find easier the substr
         httpMessageBuffer.append(recvBuffer, readBytes );
         auto found = httpMessageBuffer.find("\r\n\r\n");
         if( found != std::string::npos )
         {
            // We found it...
            // Now we can start to build our httpMessage
            break;
         }


         memset( recvBuffer, 0, BUFFER_SIZE );
         readBytes = 0;
      }

      printf( "Read: %zu bytes\n", httpMessageBuffer.size() );
      printf( "Received: %s\n", httpMessageBuffer.c_str() );

      return Request_t();
   }

   //-----------------------------------------------------------------------------
   void SocketIOHandler_t::writeHTTPMessage( int socketFD, const Request_t& request )
   {
      // TODO
   }
};