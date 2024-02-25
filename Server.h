#pragma once

// System Headers
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

namespace SandServer
{

// Setting backlog for listen call
constexpr int32_t NUM_K_EVENTS = 15;
constexpr int32_t BACK_LOG     = 10;
constexpr int32_t NUM_WORKERS  = 5;

class Server_t
{
   
   public:
      Server_t();
      ~Server_t() = default;

      bool start( int32_t port );
      void listenAndAccept();

    private:
      struct addrinfo hints, *servinfo, *p;
      // Strucutre holder information on how to act and handle signals from
      // childs
      struct sigaction sa;

      int socketFd;

      // Kqueue stuff
      int kq;
      int workerKqueueFD[ NUM_WORKERS ];

      // Threading baby
      std::thread listenerThread;
      struct kevent wrkEvents[ NUM_WORKERS ][ NUM_K_EVENTS ];
      struct kevent wrkChangedEvents[ NUM_WORKERS ][ NUM_K_EVENTS ];
};
};
