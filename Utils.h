#pragma once

// Project Headers
#include "Common.h"
#include "Connection.h"

namespace SandServer
{
namespace Utils
{
//-----------------------------------------------------------------------------
/// Function to check if time elapsed based on lastActivity
/// @param conn holding lastActivity member
bool timeoutElapsed( const Connection_t& conn );

//-----------------------------------------------------------------------------
/// Function to split string by delimiter
StringContainer_t splitString( const std::string& str, const char delimiter );

};   // namespace Utils
};   // namespace SandServer
