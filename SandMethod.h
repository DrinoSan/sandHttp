#pragma once

#include "Log.h"

namespace SandServer
{
enum class HttpMethod
{
   GET,
   POST,
   PUT,
   DELETE,
   UNKNOWN
};

namespace httpMethod
{
HttpMethod stringToMethod( const std::string& method );
std::string methodToString( const HttpMethod& method );
}   // namespace httpMethod
};   // namespace SandServer
