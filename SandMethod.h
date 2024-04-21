#pragma once

#include "Log.h"

namespace SandServer
{
   enum class SAND_METHOD
   {
      GET,
      POST,
      PUT,
      DELETE,
      UNKNOWN
   };

   SAND_METHOD stringToMethod( const std::string& method );
   std::string methodToString( const SAND_METHOD& method );
};

