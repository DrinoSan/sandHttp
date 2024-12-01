#include <iostream>   // Needed to print config. SLOG does not like tbl

#include "config.h"
#define TOML_EXCEPTIONS 1
#include "Log.h"
#include "toml.h"
#include "Exceptions.h"

namespace SandServer
{
ServerConfig_t::ServerConfig_t( std::string& configPath_ )
    : configPath{ std::move( configPath_ ) }
{
   SLOG_INFO( "Config Parsing initialized" );
}

// ----------------------------------------------------------------------------
bool ServerConfig_t::parse()
{
   try
   {
      tbl = toml::parse_file( configPath );
      std::cout << "File parsed successfully " << tbl << "\n";
   }
   catch ( const toml::parse_error& err )
   {
      SLOG_ERROR( "Parsing Failed: {0}", err.what() );
      throw std::runtime_error( "Error parsing file: config.toml" );
   }

   if ( !tbl[ "server" ][ "host" ] || !tbl[ "server" ][ "port" ] ||
        !tbl[ "server" ][ "num_k_events" ] ||
        !tbl[ "server" ][ "back_log" ] || !tbl[ "server" ][ "num_workers" ] )
   {
      SLOG_ERROR( "CONFIG FILE WRONG FORMATTED CHECK VALUES" );
      SLOG_INFO( "Example Config: \n{0}", "\nlevel = 'info'"
                                          "\nlog_file = 'server.log'"

                                          "\n[security]"
                                          "\nenable_ssl = false"
                                          "\nssl_cert = 'server.crt'"
                                          "\nssl_key = 'server.key'"

                                          "\n[server]"
                                          "\nbacklog = 10"
                                          "\nhost = '127.0.0.1'"
                                          "\nmax_connections = 100"
                                          "\nnum_k_events = 100"
                                          "\nnum_workers = 5"
                                          "\nport = 8080"

                                          "\n[static_files]"
                                          "\nserve_static = true"
                                          "\nstatic_dir = './public'" );

      SLOG_WARN("Default values for config will be used");
   }

   // Not sure if i should throw if a value does not exist...
   host         = tbl[ "server" ][ "host" ].value_or( "127.0.0.1" );
   port         = tbl[ "server" ][ "port" ].value_or( "8080" );
   num_k_events = tbl[ "server" ][ "num_k_events" ].value_or( 100 );
   back_log     = tbl[ "server" ][ "back_log" ].value_or( 10 );
   num_workers  = tbl[ "server" ][ "num_workers" ].value_or( 5 );

   return true;
}


// ----------------------------------------------------------------------------
void ServerConfig_t::dump()
{
	SLOG_WARN("-------------------- DUMP CONFIG START --------------------");
	SLOG_WARN("\nHost: {0}\nPort : {1}\nNum_k_events: {2}\nBack_log: {3}\nNum_Workers: {4}\n", host, port, num_k_events, back_log, num_workers);
	SLOG_WARN("--------------------  DUMP CONFIG END  --------------------");
}
};   // namespace SandServer
