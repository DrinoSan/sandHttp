#pragma once

// System Headers
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <netdb.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <utility>
#include <vector>

// Project Headers
#include "Connection.h"
#include "HttpMessage.h"
#include "Router.h"
#include "SandMethod.h"
#include "SocketHandler.h"
#include "SocketIOHandler.h"
#include "ThreadPool.h"
#include "config/config.h"

namespace fs = std::filesystem;

namespace SandServer
{
using handlerFunc =
    std::function<void( HTTPRequest_t& request, HTTPResponse_t& response )>;

struct RouteKey
{
   std::string uri;
   SAND_METHOD method;
   bool        isPattern;
   std::string pathValuePlaceholder;

   // Just playing
   std::string placeholder;
   int32_t     placeholderBegin;

   RouteKey( std::string uri_, const std::string& method_,
             bool isPattern_ = false, std::string pathPlaceholder_ = "" )
       : uri( std::move( uri_ ) ),
         method( httpMethod::stringToMethod( method_ ) ),
         isPattern( isPattern_ ),
         pathValuePlaceholder( std::move( pathPlaceholder_ ) )
   {
   }

   // When you already coding please implement a json parser for the messages
   // Also check if the http message parser in socketIOhandler can handle
   // requests with a body / content length Function to provide ability for
   // usage as key in map
   bool operator<( const RouteKey& other ) const
   {
      if ( this->uri != other.uri )
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
   Server_t( std::string configPath );
   ~Server_t();
   SocketIOHandler_t socketIOHandler;
   SocketHandler_t   socketHandler;

   // Function to handle static files served from filePath
   /// @param filePath path of files to be served from
   /// @param urlPrefix on which endpoint url the files will be served
   void serveStaticFiles( std::string_view filePath,
                          std::string_view urlPrefix );

   // Function to start the server
   bool start( int32_t port_ = 0 );

   // Function to add routes which will be handeld by the server
   // @param route is the endpoint which should be handeld
   // @param method is the method type for which this endpoint should be
   // usable
   // @param function to be executed on endpoint call
   bool addRoute( std::string&& route, const SAND_METHOD& method,
                  HandlerFunc handler );

 private:
   // Function to accept incoming connection wheter ipv6 or ipv4
   void listenAndAccept();

   // Function to check if a connection has reached the timout
   bool timeoutElapsed( const Connection_t& conn );

   // Work baby work
   void processWorkerEvents( int32_t workerIdx );

   // Function to handle incoming requests
   /// Handling keep-alive for now
   std::pair<HTTPResponse_t, bool>
   generateResponse( HTTPRequest_t& httpRequest );

   // Function to handle incoming routes from clients
   // TODO: Later this function will probably return something
   // @param incoming request
   auto handleRouting( HTTPRequest_t& request ) -> handlerFunc;

   // Function to read and return file content into response
   /// @param servingDir directory which is served
   /// @param file/resource which will be served and returned in response body
   /// @return HTTPResponse_t which file content loaded in body and headers set
   /// appropriate to file extension
   HTTPResponse_t serveFile( const fs::path&                 servingDir,
                             const std::vector<std::string>& urlParts );

 private:
   struct addrinfo hints, *servinfo, *p;
   // Strucutre holder information on how to act and handle signals from
   // childs
   struct sigaction sa;
   struct sigaction saInter;

   int           socketFd;
   volatile bool isRunning{ true };

   // Threading baby
   std::thread  listenerThread;
   ThreadPool_t threadPool;

   int32_t readAll( int32_t sockFd );

   // Could also use std::map<std::tuple<std::string, SAND_METHOD>,
   // std::function<void()>> routes;
   std::map<RouteKey, handlerFunc> routes;

   Router_t       router;
   ServerConfig_t config;
};
};   // namespace SandServer
