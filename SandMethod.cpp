// Project Headers
#include "SandMethod.h"
#include "Log.h"

namespace SandServer
{
namespace httpMethod
{
//-----------------------------------------------------------------------------
HttpMethod stringToMethod( const std::string& method )
{
   if ( method == "GET" )
   {
      return HttpMethod::GET;
   }
   if ( method == "POST" )
   {
      return HttpMethod::POST;
   }
   if ( method == "PUT" )
   {
      return HttpMethod::PUT;
   }
   if ( method == "DELETE" )
   {
      return HttpMethod::DELETE;
   }

   SLOG_INFO( "Unknown HTTP method" );
   return HttpMethod::UNKNOWN;
}

//-----------------------------------------------------------------------------
std::string methodToString( const HttpMethod& method )
{
   switch ( method )
   {
   case HttpMethod::GET:
      return "GET";

   case HttpMethod::POST:
      return "POST";

   case HttpMethod::PUT:
      return "PUT";

   case HttpMethod::DELETE:
      return "DELETE";

   default:
      SLOG_ERROR( "Got unknown http method" );
      return "UNKNOWN METHOD";
   }
}
};   // namespace httpMethod
};   // namespace SandServer
