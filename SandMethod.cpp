// Project Headers
#include "SandMethod.h"

namespace SandServer
{
//-----------------------------------------------------------------------------
SAND_METHOD stringToMethod( const std::string& method )
{
    if ( method == "GET" )
    {
        return SAND_METHOD::GET;
    }
    else if ( method == "POST" )
    {
        return SAND_METHOD::POST;
    }
    else if ( method == "PUT" )
    {
        return SAND_METHOD::PUT;
    }
    else if ( method == "DELETE" )
    {
        return SAND_METHOD::DELETE;
    }

    SLOG_INFO( "Unknown HTTP method" );
    return SAND_METHOD::UNKNOWN;
}
};   // namespace SandServer
