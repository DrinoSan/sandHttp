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
#include <signal.h>

// Project Headers
#include "Connection.h"
#include "HttpMessage.h"
#include "ProtocolHandler.h"
#include "Router.h"
#include "SandMethod.h"
#include "SocketHandler.h"
#include "ThreadPool.h"
#include "config/config.h"

namespace fs = std::filesystem;

namespace SandServer
{
class Server_t
{
 public:
   Server_t();
   Server_t( std::string configPath );
   ~Server_t();
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
   void addRoute( std::string&& route, const HttpMethod& method,
                  HandlerFunc handler );

 private:
   // Function to detect which protocol should be used
   ProtocolType_t detectProtocol( const std::string& data );

   // Function to accept incoming connection wheter ipv6 or ipv4
   void listenAndAccept();

   // Function to check if a connection has reached the timout
   bool timeoutElapsed( const Connection_t& conn );

   // Work baby work
   void processWorkerEvents( int32_t workerIdx );

   // Function to handle incoming routes from clients
   // TODO: Later this function will probably return something
   // @param incoming request
   HandlerFunc handleRouting( HTTPRequest_t& request );

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

   volatile bool isRunning{ true };

   // Threading baby
   std::thread  listenerThread;
   ThreadPool_t threadPool;

   Router_t       router;
   ServerConfig_t config;
};
};   // namespace SandServer
