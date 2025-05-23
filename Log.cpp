// Project Headers
#include "Log.h"

namespace SandServer
{
std::shared_ptr<spdlog::logger> Log_t::s_CoreLogger;

void Log_t::init()
{
   // spdlog::set_pattern( "%^[%T] %v%$ [%s:%!: %#]" );
   // spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
   spdlog::set_pattern( "[%H:%M:%S %z] [%^%L%$] %v" );
   spdlog::stdout_color_mt( "SandServer" );

   s_CoreLogger = spdlog::get( "SandServer" );
}
}   // namespace SandServer
