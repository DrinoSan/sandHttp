#pragma once
#include <iostream>
#include <stdexcept>

namespace SandServer
{
// ----------------------------------------------------------------------------
// Config Exceptions
class ConfigMissingField : public std::exception
{
 public:
   ConfigMissingField( char* msg ) : message( msg ) {}
   char* what() { return message; }

 private:
   char* message;
};

// ----------------------------------------------------------------------------
// Socket exception timout based to close dead sockets
class TimeoutException : public std::runtime_error
{
 public:
   TimeoutException( const std::string& message )
       : std::runtime_error( message )
   {
   }
};

// ----------------------------------------------------------------------------
// Client sent 0 byets we need to close the socket
class ClientClosedConnectionException : public std::runtime_error
{
 public:
   ClientClosedConnectionException( const std::string& message )
       : std::runtime_error( message )
   {
   }
};

// ----------------------------------------------------------------------------
// I have no idea what i am catching
/// The name is long and will piss me off
class UnexpectedSocketException
    : public std::runtime_error
{
 public:
   UnexpectedSocketException(
       const std::string& message )
       : std::runtime_error( message )
   {
   }
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// -------------------------- PARSING EXCEPTIONS ------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class ParsingException_t : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// ----------------------------------------------------------------------------
// Client sent 0 byets we need to close the socket
class RequestHeaderException : public ParsingException_t
{
 public:
   RequestHeaderException( const std::string& message )
       : ParsingException_t( message )
   {
   }
};

// ----------------------------------------------------------------------------
// Client send invalid statusline
class ParsingExceptionMissingRequestline_t : public ParsingException_t
{
 public:
   ParsingExceptionMissingRequestline_t( const std::string& message )
       : ParsingException_t( message )
   {
   }
};

// ----------------------------------------------------------------------------
// Client send invalid request. Missing \r\n\r\n
class ParsingExceptionMissingHeaderDelimiter_t : public ParsingException_t
{
 public:
   ParsingExceptionMissingHeaderDelimiter_t( const std::string& message )
       : ParsingException_t( message )
   {
   }
};

// ----------------------------------------------------------------------------
// Client send invalid request. Missing \r\n\r\n
class ParsingExceptionMalformedStatusLine_t : public ParsingException_t
{
 public:
   ParsingExceptionMalformedStatusLine_t( const std::string& message )
       : ParsingException_t( message )
   {
   }
};

// ----------------------------------------------------------------------------
// Client send invalid request. Missing \r\n\r\n
class ParsingExceptionUnsupportedHTTPMethod_t : public ParsingException_t
{
 public:
   ParsingExceptionUnsupportedHTTPMethod_t( const std::string& message )
       : ParsingException_t( message )
   {
   }
};

// ----------------------------------------------------------------------------
// Client send invalid request. Missing \r\n\r\n
class ParsingExceptionMalformedHeader_t : public ParsingException_t
{
 public:
   ParsingExceptionMalformedHeader_t( const std::string& message )
       : ParsingException_t( message )
   {
   }
};
};   // namespace SandServer
