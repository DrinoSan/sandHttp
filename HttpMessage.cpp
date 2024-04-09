// Project Headers
#include "HttpMessage.h"
#include "Log.h"

namespace SandServer
{
void HTTPMessage_t::setHeader( const std::string& name,
                               const std::string& value )
{
    headers[ name ] = value;
}

void HTTPMessage_t::printHeaders()
{
    SLOG_INFO( "DUMP HEADERS" );
    for ( const auto& [ k, v ] : headers )
    {
        SLOG_INFO( "Key: {0} Value: {1}", k, v );
    }
}
};   // namespace SandServer
