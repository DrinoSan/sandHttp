#pragma once

// System Headers
#include <cstdint>
#include <functional>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

// Project Headers
#include "SandMethod.h"
#include "HttpMessage.h"

namespace SandServer
{

// Setting backlog for listen call
// TODO: make this change able in config file
constexpr int32_t NUM_K_EVENTS = 100;
constexpr int32_t BACK_LOG     = 10;
constexpr int32_t NUM_WORKERS  = 5;

struct RouteKey
{
   std::string uri;
   SAND_METHOD method;

   RouteKey( const std::string& uri_, const std::string& method_ )
   {
      method = stringToMethod( method_ );
      uri  = uri_;
   }

   // When you already coding please implement a json parser for the messages
   // Also check if the http message parser in socketIOhandler can handle requests with a body / content length
   // Function to provide ability for usage as key in map
   bool operator<(const RouteKey &other) const
   {
      if( this->uri != other.uri )
      {
         return this->uri < other.uri;
      }

      return this->method < other.method;
   }
};

class Server_t
{

   public:
      Server_t();
      ~Server_t() = default;

      bool start( int32_t port );

      private:
      // Function to accept incoming connection wheter ipv6 or ipv4
      void listenAndAccept();

      // Work baby work
      void processWorkerEvents( int32_t workerIdx );

      // Function to handle incoming routes from clients
      // TODO: Later this function will probably return something
      // @param incoming request
      void handleRouting( const HTTPRequest_t& request );

      // Function to add routes which will be handeld by the server
      // @param route is the endpoint which should be handeld
      // @param method is the method type for which this endpoint should be
      // usable
      // @param function to be executed on endpoint call
      void addRoute( const std::string& route, const SAND_METHOD& method,
                     std::function<void( HTTPRequest_t&  request,
                                         HTTPResponse_t& response )>
                         handler );

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

      // Could also use std::map<std::tuple<std::string, SAND_METHOD>, std::function<void()>> routes;
      std::map<RouteKey, std::function<void( HTTPRequest_t&, HTTPResponse_t& )>>
          routes;
};
};
