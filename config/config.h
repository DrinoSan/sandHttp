#pragma once

#include <cstdint>
#include <string>

#include "toml.h"

namespace SandServer
{
class ServerConfig_t
{
 public:
   ServerConfig_t();
   explicit ServerConfig_t( std::string& configPath_ );
   ServerConfig_t( const ServerConfig_t& )            = delete;
   ServerConfig_t( ServerConfig_t&& )                 = delete;
   ServerConfig_t& operator=( const ServerConfig_t& ) = delete;
   ServerConfig_t& operator=( ServerConfig_t&& )      = delete;
   ~ServerConfig_t()                                  = default;

   bool parse();
   void dump();

   std::string host;
   std::string port;
   int32_t     back_log;
   int32_t     num_workers;

 private:
   toml::table tbl;
   std::string configPath;
};

};   // namespace SandServer
