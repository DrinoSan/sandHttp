#include <iostream>

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
};   // namespace SandServer
