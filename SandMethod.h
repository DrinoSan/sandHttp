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

namespace httpMethod
{
SAND_METHOD stringToMethod( const std::string& method );
std::string methodToString( const SAND_METHOD& method );
}   // namespace httpMethod
};   // namespace SandServer
