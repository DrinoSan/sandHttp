// System Headers
#include <memory>

// Vendor Headers
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace SandServer
{
   class Log_t
   {
      public:
         static void init();

         inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
         {
             return s_CoreLogger;
         }

     private:
       static std::shared_ptr<spdlog::logger> s_CoreLogger;
   };
};


// Core Log Macros
// SLOG Server Log initially i thought SS_LOG_FATAL, SS_LOG_ERROR and so on but SS is wierd
#define SLOG_FATAL( ... ) ::SandServer::Log_t::GetCoreLogger()->fatal( __VA_ARGS__ )
#define SLOG_ERROR( ... ) ::SandServer::Log_t::GetCoreLogger()->error( __VA_ARGS__ )
#define SLOG_WARN( ... )  ::SandServer::Log_t::GetCoreLogger()->warn( __VA_ARGS__ )
#define SLOG_INFO( ... )  ::SandServer::Log_t::GetCoreLogger()->info( __VA_ARGS__ )
#define SLOG_TRACE( ... ) ::SandServer::Log_t::GetCoreLogger()->trace( __VA_ARGS__ )

