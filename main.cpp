// System HEADERS
#include <boost/asio.hpp>

// Project HEADERS
#include "Server.h"
#include "Log.h"

using namespace boost;

int main(void)
{
   SandServer::Log_t::init();
   SLOG_WARN("Initialized Log!");

   asio::io_service ios;
   SandServer::Server_t server;

   server.start( 8000 );

   return 0;
}
