// Project Headers
#include "HttpMessage.h"
#include "Log.h"

namespace SandServer
{
//-----------------------------------------------------------------------------
void HTTPMessage_t::setHeader( const std::string& name,
                               const std::string& value )
{
    headers[ name ] = value;
}

//-----------------------------------------------------------------------------
void HTTPMessage_t::printHeaders()
{
    SLOG_INFO( "DUMP HEADERS" );
    for ( const auto& [ k, v ] : headers )
    {
        SLOG_INFO( "Key: {0} Value: {1}", k, v );
    }

    SLOG_INFO( "HTTPMessage Body: {0}", body );
}

//-----------------------------------------------------------------------------
void HTTPMessage_t::printObject()
{
    SLOG_TRACE( "-------------------  DUMP OBJECT BEGIN ------------------- " );
    printHeaders();
    SLOG_TRACE( "------------------- DUMP OBJECT END ------------------- " );
}

//-----------------------------------------------------------------------------
void HTTPRequest_t::printObject()
{
    HTTPMessage_t::printObject();
    SLOG_TRACE( "Method: {0}", method );
    SLOG_TRACE( "URI: {0}", uri );
    SLOG_TRACE( "Version: {0}", version );
}

//-----------------------------------------------------------------------------
void HTTPResponse_t::printObject()
{
    HTTPMessage_t::printObject();
    SLOG_TRACE( "StatusCode: {0}", statusCode );
}
};   // namespace SandServer
