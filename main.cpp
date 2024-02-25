// System HEADERS
#include <boost/asio.hpp>

// Project HEADERS
#include "Server.h"

using namespace boost;

int main(void)
{
   asio::io_service ios;
   SandServer::Server_t server;

   server.start( 8000 );

   return 0;
}
