
#pragma once

// System Headers
#include <string>
#include <sys/socket.h>
#include <netdb.h>


namespace SandServer
{
class Server_t
{
   
   public:
      Server_t();
      ~Server_t() = default;

      bool start( const std::string& ipAdr, int32_t port );
   
   private:
   struct addrinfo hints, *servinfo, *p;
};
};
