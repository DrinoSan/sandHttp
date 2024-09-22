#include <iostream>

#include "config.h"
#define TOML_EXCEPTIONS 1
#include "Log.h"
#include "toml.h"

namespace SandServer
{
ServerConfig_t::ServerConfig_t( std::string& configPath_ )
    : configPath{ std::move( configPath_ ) }
{
    SLOG_INFO( "Config Parsing initialized" );
}

bool ServerConfig_t::parse()
{
    try
    {
        tbl = toml::parse_file( configPath );
        std::cout << "File parsed successfully: " << tbl << "\n";
    }
    catch ( const toml::parse_error& err )
    {
        std::cerr << "Parsing failed:\n" << err << "\n";
        throw std::runtime_error( "Error parsing file: config.toml" );
    }

    host         = tbl[ "server" ][ "host" ].value_or( "127.0.0.1" );
    port         = tbl[ "server" ][ "port" ].value_or( "8080" );
    num_k_events = tbl[ "server" ][ "num_k_events" ].value_or( 100 );
    back_log     = tbl[ "server" ][ "back_log" ].value_or( 10 );
    num_workers  = tbl[ "server" ][ "num_workers" ].value_or( 5 );

    return true;
}
};   // namespace SandServer
