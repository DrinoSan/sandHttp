// System Headers
#include <algorithm>
#include <utility>

// Project Headers
#include "Log.h"
#include "Router.h"
#include "SandMethod.h"

namespace SandServer
{

// Defined in utils.cpp
std::vector<std::string> splitString( const std::string& str, char delimiter );

//-----------------------------------------------------------------------------
Route_t::Route_t( std::string route_, HandlerFunc handler_,
                  SAND_METHOD httpMethod_ )
    : route{ std::move( route_ ) }, handler{ std::move( handler_ ) },
      httpMethod{ httpMethod_ }
{
    routeElements = splitString( route, '/' );
    size_t pos    = 0;

    // TODO: Check if it also works with missing '}'
    while ( ( pos = route.find( '{', pos ) != std::string::npos ) )
    {
        size_t end = route.find( '}', pos );
        if ( end != std::string::npos )
        {
            placeholders.push_back(
                route.substr( pos + 1, end - pos - 1 ) );   // cutting {}
            pos = end + 1;
        }
    }
}

//-----------------------------------------------------------------------------
std::optional<HandlerFunc> Router_t::matchRoute( HTTPRequest_t& request )
{
    SLOG_INFO( "Entering matchRoute" );

    auto requestPathElements = splitString( request.getURI(), '/' );

    if ( requestPathElements[ 0 ] == staticFilesUrlPrefix &&
         request.getMethod() == SAND_METHOD::GET )
    {
        request.setUrlParts( requestPathElements );
        return staticFileHandler;
    }

    for ( const auto& route : routes )
    {
        if ( route.routeElements.size() != requestPathElements.size() )
        {
            continue;
        }

        if ( route.httpMethod != request.getMethod() )
        {
            continue;
        }

        bool match{ true };
        for ( size_t i = 0; i < route.routeElements.size(); ++i )
        {
            if ( route.routeElements[ i ][ 0 ] == '{' &&
                 route.routeElements[ i ].back() == '}' )
            {
                std::string placeholder = route.routeElements[ i ].substr(
                    1, route.routeElements[ i ].size() - 2 );

                request.query[ placeholder ] = requestPathElements[ i ];
                SLOG_ERROR( "Adding placeholder {0} with value {1}",
                            placeholder, requestPathElements[ i ] );
            }
            else if ( route.routeElements[ i ] != requestPathElements[ i ] )
            {
                match = false;
                break;
            }
        }

        if ( match )
        {
            return route.handler;
        }
    }

    SLOG_INFO( "No route found" );

    return {};
}

}   // namespace SandServer
