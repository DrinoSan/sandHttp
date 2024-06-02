// System Headers
#include <fstream>
#include <string>

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

//-----------------------------------------------------------------------------
void HTTPResponse_t::notFound()
{
    std::ifstream page404( "html/notFound.html" );
    if ( !page404.is_open() )
    {
        SLOG_ERROR( "Cant read 404 page from file" );
        return;
    }

    // HTTP response with a simple "Hello World" message
    // This is only for test to get a real response from server
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    std::string line;
    while ( getline( page404, line ) )
    {
        body += line;
    }
    
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "\r\n";

    body = response + body;
    // Close the file
    page404.close();
}
};   // namespace SandServer
