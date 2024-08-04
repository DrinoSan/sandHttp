// Project Headers
#include "SandMethod.h"
#include "Log.h"

namespace SandServer
{
namespace httpMethod
{
//-----------------------------------------------------------------------------
SAND_METHOD stringToMethod( const std::string& method )
{
    if ( method == "GET" )
    {
        return SAND_METHOD::GET;
    }
    if ( method == "POST" )
    {
        return SAND_METHOD::POST;
    }
    if ( method == "PUT" )
    {
        return SAND_METHOD::PUT;
    }
    if ( method == "DELETE" )
    {
        return SAND_METHOD::DELETE;
    }

    SLOG_INFO( "Unknown HTTP method" );
    return SAND_METHOD::UNKNOWN;
}

//-----------------------------------------------------------------------------
std::string methodToString( const SAND_METHOD& method )
{
    switch ( method )
    {
    case SAND_METHOD::GET:
        return "GET";

    case SAND_METHOD::POST:
        return "POST";

    case SAND_METHOD::PUT:
        return "PUT";

    case SAND_METHOD::DELETE:
        return "DELETE";

    default:
        SLOG_ERROR( "Got unknown http method" );
        return "UNKNOWN METHOD";
    }
}
};   // namespace httpMethod
};   // namespace SandServer
