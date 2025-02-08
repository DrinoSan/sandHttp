#pragma once

// System Headers
#include <string.h>

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
   static int32_t readFromSocket( Connection_t& conn );
   static void    writeToSocket( Connection_t& conn, const std::string& data );

   // Function to check if there is any data to read on socketFD
   [[nodiscard]] static bool hasIncomingData( int socketFD );
};

};   // namespace SandServer
