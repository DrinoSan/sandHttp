#pragma once

// System Headers
#include <memory>

// Vendor Headers
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

namespace SandServer
{
class Log_t
{
 public:
   // Not sure if i need to delete the constructors and i guess i need to delete
   // the others too
   Log_t()             = delete;
   Log_t( Log_t& log ) = delete;

   static void init();

   inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
   {
      return s_CoreLogger;
   }

 private:
   // Because static we only need to call init once in the main
   static std::shared_ptr<spdlog::logger> s_CoreLogger;
};
};   // namespace SandServer

// Core Log Macros
// SLOG Server Log initially i thought SS_LOG_FATAL, SS_LOG_ERROR and so on but
// SS is wierd
//#define SLOG_FATAL( ... )                                                      \
//   SPDLOG_LOGGER_FATAL( ::SandServer::Log_t::GetCoreLogger(), __VA_ARGS__ )
//#define SLOG_FATAL( ... ) ::SandServer::Log_t::GetCoreLogger()->fatal( __VA_ARGS__ )
#define SLOG_FATAL(...) \
    SPDLOG_LOGGER_CALL(::SandServer::Log_t::GetCoreLogger(), spdlog::level::critical, __VA_ARGS__)

//#define SLOG_ERROR( ... )                                                      \
//   SPDLOG_LOGGER_ERROR( ::SandServer::Log_t::GetCoreLogger(), __VA_ARGS__ )
#define SLOG_ERROR( ... ) ::SandServer::Log_t::GetCoreLogger()->error( __VA_ARGS__ )
//#define SLOG_WARN( ... )                                                       \
//   SPDLOG_LOGGER_WARN( ::SandServer::Log_t::GetCoreLogger(), __VA_ARGS__ )
#define SLOG_WARN( ... )  ::SandServer::Log_t::GetCoreLogger()->warn( __VA_ARGS__ )
//#define SLOG_INFO( ... )                                                       \
//   SPDLOG_LOGGER_INFO( ::SandServer::Log_t::GetCoreLogger(), __VA_ARGS__ )
#define SLOG_INFO( ... ) ::SandServer::Log_t::GetCoreLogger()->info( __VA_ARGS__ )
//#define SLOG_TRACE( ... )                                                      \
//   SPDLOG_LOGGER_TRACE( ::SandServer::Log_t::GetCoreLogger(), __VA_ARGS__ )
#define SLOG_TRACE( ... ) ::SandServer::Log_t::GetCoreLogger()->trace( __VA_ARGS__ )

