// System Headers
#include <string.h>
#include <sys/socket.h>
#include <vector>

// Project Headers
#include "HttpMessage.h"
#include "Log.h"
#include "SocketIOHandler.h"

namespace SandServer
{

//-----------------------------------------------------------------------------
/// Function to parse a raw string into a HTTPRequest_t
/// @param msg pointer to the start of the string
/// @param msgEnd pointer to the end of the string
/// @return HTTPRequest_t object
HTTPRequest_t parseRawString( const char* msg, const char* msgEnd )
{
    HTTPRequest_t request;

    const char* head = msg;
    const char* tail = msg;

    // -------------------- Find request type --------------------
    while ( tail != msgEnd && *tail != ' ' )
    {
        ++tail;
    }
    request.setMethod( std::string( head, tail ) );

    // -------------------- Find path --------------------
    while ( tail != msgEnd && *tail == ' ' )
    {
        ++tail;   // Skipping possible whitespaces
    }

    head = tail;

    while ( tail != msgEnd && *tail != ' ' )
    {
        ++tail;
    }
    request.setURI( std::string( head, tail ) );

    // -------------------- Find HTTP version --------------------
    while ( tail != msgEnd && *tail == ' ' )
    {
        ++tail;   // Skipping possible whitespaces
    }

    head = tail;

    while ( tail != msgEnd && *tail != '\r' )
    {
        ++tail;
    }
    request.setVersion( std::string( head, tail ) );

    // -------------------- Parsing headers --------------------
    if ( tail != msgEnd )
    {
        ++tail;   // Skipping '\r'
    }
    head = tail;

    while ( head != msgEnd && *head != '\r' )
    {
        while ( tail != msgEnd && *tail != '\r' )
        {
            ++tail;
        }

        const char* colon = head;
        while ( colon != tail && *colon != ':' )
        {
            ++colon;
        }
        if ( *colon != ':' )
        {
            // This header seems insane :)
            SLOG_INFO( "ERROR" );
            break;
        }

        // TODO: SOMEWHER here is a bug....
        const char* value = colon + 1;
        while ( value != tail && *value == ' ' )
        {
            ++value;
        }

        request.setHeader( std::string( head + 1, colon ),
                           std::string( value, tail ) );
        head = ++tail;   // If we dont move taile one position up then we tail
                         // is < head and we get some errors on setHeaders call.
    }

    request.printObject();
    return request;
}

//-----------------------------------------------------------------------------
HTTPRequest_t SocketIOHandler_t::readHTTPMessage( int socketFD )
{
    std::string rawString = readFromSocket( socketFD );

    // Parse rawString
    // To get headers and everything
    return parseRawString( rawString.c_str(),
                           rawString.c_str() + rawString.size() );
}

//-----------------------------------------------------------------------------
std::string SocketIOHandler_t::readFromSocket( int socketFD )
{
    // TODO: make BUFFER_SIZE configurable
    constexpr int32_t BUFFER_SIZE{ 1024 };
    char              recvBuffer[ BUFFER_SIZE ];
    memset( recvBuffer, 0, BUFFER_SIZE );

    std::string rawData;
    int32_t     readBytes{ 0 };

    // When is a full message read
    // 1) We get a /r/n/r/n
    // 2) We read 0 bytes
    while ( ( readBytes = recv( socketFD, recvBuffer, BUFFER_SIZE, 0 ) ) )
    {
        if ( readBytes < 0 )
        {
            // We got a error
            break;
        }

        // C++ style to find easier the substr
        rawData.append( recvBuffer, readBytes );
        auto found = rawData.find( "\r\n\r\n" );
        if ( found != std::string::npos )
        {
            // We found it...
            // Now we can start to build our httpMessage
            break;
        }

        memset( recvBuffer, 0, BUFFER_SIZE );
        readBytes = 0;
    }

    return rawData;
}

// TODO: Only send CHUNK_SIZE bytes over network
//-----------------------------------------------------------------------------
void SocketIOHandler_t::writeHTTPMessage( int                   socketFD,
                                          const HTTPResponse_t& response )
{
    // Send the response
    SLOG_WARN("RESPONSE BODY: {0}", response.getBody() );
    int bytesSent = send( socketFD, response.getBody().c_str(),
                          response.getBody().size(), 0 );
    if ( bytesSent == -1 )
    {
        // Handle send error
        perror( "send" );
    }
    else
    {
        SLOG_INFO( "Sent {0} bytes to client on socket {1}", bytesSent,
                   socketFD );
    }
}
};   // namespace SandServer
