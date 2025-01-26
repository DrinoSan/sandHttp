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
};   // namespace SandServer
