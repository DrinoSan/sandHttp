#pragma once

// Project Headers
#include "Connection.h"
#include "HttpMessage.h"

namespace SandServer
{
class SocketIOHandler_t
{
 public:
   // Function to read full http message from socket
   /// @param socketFD is the socketfd to read from
   /// @return we return a Request object
   [[nodiscard]] HTTPRequest_t readHTTPMessage( Connection_t& conn );

   // Function to write a http message to client/socket
   /// @param socketFD to write data to
   /// @param httpMessage the message we want to write to the socket
   void writeHTTPMessage( Connection_t& conn, const HTTPResponse_t& response );

   // Function to check if socket has data to read
   bool hasSocketDataToRead( int32_t socketFD );
};
};   // namespace SandServer
