// Project Headers
#include "Log.h"

// Vendor Headers
#include "spdlog/sinks/stdout_color_sinks.h"

namespace SandServer
{
std::shared_ptr<spdlog::logger> Log_t::s_CoreLogger;

void Log_t::init()
{
    spdlog::set_pattern( "%^[%T] %v%$ [%s:%!: %#]" );
    spdlog::stdout_color_mt( "SandServer" );

    s_CoreLogger = spdlog::get( "SandServer" );
}
}   // namespace SandServer
