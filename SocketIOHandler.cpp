// System Headers
#include <vector>
#include <sys/socket.h>

// Project Headers
#include "Log.h"
#include "SocketIOHandler.h"

namespace SandServer
{
   //-----------------------------------------------------------------------------
HTTPRequest_t SocketIOHandler_t::readHTTPMessage( int socketFD )
{
    constexpr int32_t BUFFER_SIZE{ 1024 };
    char              recvBuffer[ BUFFER_SIZE ];
    memset( recvBuffer, 0, BUFFER_SIZE );

    std::string httpMessageBuffer;
    int32_t     readBytes{ 0 };

    // When is a full message read
    // 1) We get a /r/n/r/n
    // 2) We read 0 bytes
    while ( ( readBytes = recv( socketFD, recvBuffer, BUFFER_SIZE, 0 ) ) )
    {
        if ( readBytes < 0 )
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

      SLOG_INFO( "Read: {0} bytes", httpMessageBuffer.size() );
      SLOG_INFO( "Received: {0}", httpMessageBuffer.size() );

      return HTTPRequest_t();
   }

   //-----------------------------------------------------------------------------
   void SocketIOHandler_t::writeHTTPMessage( int                  socketFD,
                                             const HTTPRequest_t& request )
   {
      // HTTP response with a simple "Hello World" message
      // This is only for test to get a real response from server
      std::string response = "HTTP/1.1 200 OK\r\n";
      response += "Content-Type: text/plain\r\n";
      response += "Content-Length: 21\r\n";
      response += "\r\n";
      response += "Hello from SandServer";

      // Send the response
      int bytesSent = send( socketFD, response.c_str(), response.size(), 0 );
      if ( bytesSent == -1 )
      {
         // Handle send error
         perror( "send" );
      }
      else
      {
         SLOG_INFO( "Sent {0} bytes to client on socket {1}", bytesSent,
                    socketFD );
      }
   }
};
