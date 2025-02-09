#pragma once

// System Headers
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

// Project Headers
#include "Connection.h"

namespace SandServer
{

constexpr int32_t CHUNK_SIZE  = 1000;
constexpr int32_t BUFFER_SIZE = 1024;

//-----------------------------------------------------------------------------
// Handles raw socket communication
class SocketHandler_t
{
 public:
   explicit SocketHandler_t( int domain = AF_INET, int type = SOCK_STREAM,
                           int protocol = 0 );
   ~SocketHandler_t();

   // Initialize the socket: bind to an address and port, then start listening.
   // If the address is empty, it will bind to all interfaces.
   void init( const std::string& address, int32_t port, int32_t backlog = 10 );

   // Accept a new connection and return the new socket file descriptor.
   int32_t acceptConnection();

   // Close the listening socket.
   void closeSocket();

   static int32_t readFromSocket( Connection_t& conn );
   static void    writeToSocket( Connection_t& conn, const std::string& data );

   // NOT USED //
   // Function to check if there is any data to read on socketFD
   [[nodiscard]] static bool hasIncomingData( int socketFD );

   int socketFD;
};

};   // namespace SandServer
