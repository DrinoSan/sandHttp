// Project HEADERS
#include "Server.h"
#include "Log.h"

int main(void)
{
   SandServer::Log_t::init();
   SLOG_WARN("Initialized Log!");

   // asio::io_service ios;
   SandServer::Server_t server;

   server.start( 8000 );

   return 0;
}
