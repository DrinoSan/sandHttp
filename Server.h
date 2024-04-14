#pragma once

// System Headers
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <functional>
#include <map>

// Project Headers
#include "SandMethod.h"
#include "HttpMessage.h"

namespace SandServer
{

// Setting backlog for listen call
constexpr int32_t NUM_K_EVENTS = 100;
constexpr int32_t BACK_LOG     = 10;
constexpr int32_t NUM_WORKERS  = 5;

class Server_t
{

   public:
      Server_t();
      ~Server_t() = default;

      bool start( int32_t port );
      void listenAndAccept();
      void processWorkerEvents( int32_t workerIdx );

      // Routing
      void handleRouting( const HTTPRequest_t& request );

    private:
      struct addrinfo hints, *servinfo, *p;
      // Strucutre holder information on how to act and handle signals from
      // childs
      struct sigaction sa;

      int socketFd;

      // Kqueue stuff
      int kq;
      int workerKqueueFD[ NUM_WORKERS ];

      struct kevent wrkEvents[ NUM_WORKERS ][ NUM_K_EVENTS ];
      struct kevent wrkChangedEvents[ NUM_WORKERS ][ NUM_K_EVENTS ];

      // Threading baby
      std::thread listenerThread;
      std::thread workerThread[ NUM_WORKERS ];

      int32_t readAll( int32_t sockFd );

      struct RouteKey
      {
         std::string uri;
         SAND_METHOD method;

         // TODO: Implement me to be able to use me as a key in a map
         // also when you already coding please implement a json parser for the messages
         // Also check if the http message parser in socketIOhandler can handle requests with a body / content length
         bool operator<(const RouteKey &other)
         {
            return
         }
      };
      // Could also use std::map<std::tuple<std::string, SAND_METHOD>, std::function<void()>> routes;
      std::map<RouteKey, std::function<void()>> routes;
};
};
