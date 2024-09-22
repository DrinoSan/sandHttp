// System Headers
#include <fstream>
#include <sstream>
#include <string>

// Project Headers
#include "HttpMessage.h"
#include "HttpStatusCodes.h"
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
std::string HTTPMessage_t::stringifyHeaders()
{
    std::stringstream ss;
    for ( const auto& [ key, value ] : headers )
    {
        ss << key << ":" << value << "\r\n";
    }

    ss << "\r\n";

    return ss.str();
}

//-----------------------------------------------------------------------------
void HTTPMessage_t::printHeaders()
{
    for ( const auto& [ k, v ] : headers )
    {
        SLOG_INFO( "{0} : {1}", k, v );
    }

    if ( !body.empty() )
    {
        SLOG_INFO( "Body: {0}", body );
    }
}

//-----------------------------------------------------------------------------
void HTTPMessage_t::printObject()
{
    SLOG_TRACE( "-------------------  DUMP OBJECT BEGIN ------------------- " );
    printHeaders();
    SLOG_TRACE( "-------------------  DUMP OBJECT END   ------------------- " );
}

//-----------------------------------------------------------------------------
std::optional<std::string>
HTTPMessage_t::getHeader( const std::string& name ) const
{
    auto foundIt = headers.find( name );
    if ( foundIt != headers.end() )
    {
        return foundIt->second;
    }

    return {};
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HTTPRequest Definitions

//-----------------------------------------------------------------------------
std::optional<std::string>
HTTPRequest_t::pathValue( const std::string& value ) const
{
    SLOG_INFO( "Called pathValue with {0}", value );

    if ( query.find( value ) == query.end() )
    {
        SLOG_WARN( "KEY {0} not found", value );
        return {};
    }

    return query.at( value );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HTTPResponse Definitions

//-----------------------------------------------------------------------------
void HTTPRequest_t::printObject()
{
    HTTPMessage_t::printObject();
    SLOG_TRACE( "Method: {0}", method );
    SLOG_TRACE( "URI: {0}", uri );
    SLOG_TRACE( "Version: {0}", version );
}

// HTTPResponse Definitions

//-----------------------------------------------------------------------------
void HTTPResponse_t::prepareResponse()
{
    // We need:
    // 0.9 Check for body to get content-length
    if ( !body.empty() )
    {
        setHeader( "Content-Length", std::to_string( body.size() ) );
    }

    // 1 statusLine
    std::string header = setStatusLine();
    for ( const auto& [ key, value ] : headers )
    {
        header.append( key );
        header.append( ":" );
        header.append( value );
        header.append( "\r\n" );
    }

    header.append( "\r\n" );

    body = header + body;
}

//-----------------------------------------------------------------------------
void HTTPResponse_t::printObject()
{
    HTTPMessage_t::printObject();
    SLOG_INFO( "StatusCode: {0}", statusCode );
    SLOG_INFO( "ReasonPhrase: {0}", reasonPhrase );
    SLOG_INFO( "HttpVersion: {0}", httpVersion );
    SLOG_INFO( "Http Response body: {0}", body );
}

//-----------------------------------------------------------------------------
void HTTPResponse_t::notFound()
{
    std::ifstream page404( "html/notFound.html" );
    if ( !page404.is_open() )
    {
        SLOG_ERROR( "Cant read 404 page from file" );
        return;
    }

    setHeader( "Content-Type", "text/html" );
    setStatusCode( StatusCode::NotFound );
    setReasonPhraseByStatusCode( StatusCode::NotFound );

    std::string line;
    while ( getline( page404, line ) )
    {
        body += line;
    }

    // Close the file
    page404.close();
}

//-----------------------------------------------------------------------------
std::string HTTPResponse_t::stringifyHeaders()
{
    std::string header;
    for ( const auto& [ key, value ] : headers )
    {
        header.append( key );
        header.append( ":" );
        header.append( value );
        header.append( "\r\n" );
    }

    header.append( "\r\n" );

    return header;
}

};   // namespace SandServer
