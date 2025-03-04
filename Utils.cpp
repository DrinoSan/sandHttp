// System Headers
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// Project Headers
#include "Log.h"
#include "Utils.h"

namespace SandServer
{
namespace Utils
{
//-----------------------------------------------------------------------------
bool timeoutElapsed( const Connection_t& conn )
{
   // TODO make timeout setable via config
   const std::chrono::seconds IDLE_TIMEOUT( 10 );

   // Check if the connection has been idle longer than the timeout
   auto now          = std::chrono::steady_clock::now();
   auto idleDuration = now - conn.lastActivityTime;

   return idleDuration > IDLE_TIMEOUT;
}

//-----------------------------------------------------------------------------
StringContainer_t splitString( const std::string& str, const char delimiter )
{
   std::vector<std::string> res;

   const char* head = str.data();
   const char* tail = head;

   while ( true )
   {
      if ( *tail == '\0' )
      {
         SLOG_INFO( "Splitting string finished" );
         res.emplace_back( head, tail );
         break;
      }

      if ( !str.empty() && tail == str.data() && *tail == delimiter )
      {
         ++tail;
         ++head;

         continue;
      }

      if ( *tail == delimiter )
      {
         res.emplace_back( head, tail );
         head = tail + 1;
      }

      ++tail;
   }

   return res;
}

//-----------------------------------------------------------------------------
// wtf
std::vector<std::string> splitString( std::string      str,
                                      std::string_view delimiter )
{
   SLOG_ERROR( "TEST IN SPLIT STRING FUNCTION" );
   std::vector<std::string> res;

   size_t pos  = 0;
   size_t head = 0;
   size_t tail = 0;

   while ( pos < str.size() )
   {
      pos = str.find( delimiter, tail );
      SLOG_ERROR( "Found delimiter {0} starting from idxL {1} for str: {2}",
                  pos, tail, str );

      res.push_back( str.substr( head, pos ) );

      head = pos;
      tail = pos;
   }

   for ( const auto& word : res )
   {
      SLOG_ERROR( "SAND Word: {0}", word );
   }
   return res;
}

};   // namespace Utils
};   // namespace SandServer
