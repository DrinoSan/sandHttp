#pragma once

// System Headers
#include <chrono>
#include <string>

//-----------------------------------------------------------------------------
enum ConnectionState_t
{
   IDLE,
   ACTIVE,
   CLOSED
};

//-----------------------------------------------------------------------------
struct Connection_t
{
   int                                   socketFD;
   ConnectionState_t                     state;
   std::string                           persistentBuffer;
   std::chrono::steady_clock::time_point lastActivityTime;

   Connection_t( int socket )
       : socketFD{ socket }, state{ ConnectionState_t::IDLE },
         lastActivityTime{ std::chrono::steady_clock::now() }
   {
   }
};
