#pragma once

// System Headers
#include <string>


// Project Headers
#include "Request.h"



namespace SandServer
{
   class SocketIOHandler_t
   {
      public:
        // Function to read full http message from socket
        /// @param socketFD is the socketfd to read from
        /// @return we return a Request object
        /// TODO: Think to create a HTTP MESSAGE class and then child Request_t....
        [[nodiscard]]
        static Request_t readHTTPMessage( int socketFD );


        // Function to write a http message to client/socket
        /// @param socketFD to write data to
        /// @param httpMessage the message we want to write to the socket
        /// TODO: This is still wrong i need to write a http message object and not a request 
        static void writeHTTPMessage( int socketFD, const Request_t& request );
   };
};
