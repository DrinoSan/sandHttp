#pragma once

// System Headers
#include <optional>
#include <string>

// Project Headers
#include "HttpMessage.h"
#include "SandMethod.h"

namespace SandServer
{

using HandlerFunc =
    std::function<void( const HTTPRequest_t&, HTTPResponse_t& )>;

class Route_t
{
 public:
   //-----------------------------------------------------------------------------
   // Route holding information regarding route string and its handler function
   Route_t( std::string route, HandlerFunc handler, SAND_METHOD httpMethod_ );
   ~Route_t() = default;

 public:
   std::string              route;
   SAND_METHOD              httpMethod;
   HandlerFunc              handler;
   std::vector<std::string> placeholders;
   std::vector<std::string> routeElements;
};

//-----------------------------------------------------------------------------
class Router_t
{
 public:
   //-----------------------------------------------------------------------------
   /// Function to add register new routes
   /// @param route route to add
   /// @param handler function which will be executed at given route
   inline void addRoute( const std::string& route, HandlerFunc handler,
                         SAND_METHOD httpMethod )
   {
      routes.emplace_back( route, handler, httpMethod );
   }

   //-----------------------------------------------------------------------------
   std::optional<HandlerFunc> matchRoute( HTTPRequest_t& request );

 public:
   std::string_view staticFilesUrlPrefix;
   HandlerFunc      staticFileHandler;

 private:
   std::vector<Route_t> routes;
};

}   // namespace SandServer
