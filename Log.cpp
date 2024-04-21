// Project Headers
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include "Log.h"

// Vendor Headers
#include "spdlog/sinks/stdout_color_sinks.h"

namespace SandServer
{
std::shared_ptr<spdlog::logger> Log_t::s_CoreLogger;

void Log_t::init()
{
    spdlog::set_pattern( "%^[%T] %n: %v%$ [%s] [%!: %#]" );
    s_CoreLogger = spdlog::stdout_color_mt( "SandServer" );
    s_CoreLogger->set_level( spdlog::level::trace );
}
}

