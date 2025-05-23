#pragma once

// System Headers
#include <string>

// Project Headers
#include "HttpMessage.h"

namespace SandServer
{
class HttpParser_t
{
 public:
   //-----------------------------------------------------------------------------
   /// Function to parse a raw string into a HTTPRequest_t
   /// @param msg pointer to the start of the string
   /// @param msgEnd pointer to the end of the string
   /// @return HTTPRequest_t object
   static HTTPRequest_t parseRequest( const std::string& rawMessage );
};

};   // namespace SandServer
