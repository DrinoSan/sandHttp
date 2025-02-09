#pragma once

// Project Headers
#include "Connection.h"
#include "Router.h"

namespace SandServer
{
// Interface for handling different protocols
// Currently planed
// 1) Http
// 2) Websocket
enum class ProtocolType_t
{
   UNKNOWN,
   HTTP,
   WEBSOCKET,
   MQTT
};

class ProtocolHandler_t
{
 public:
   virtual ~ProtocolHandler_t() = default;
   virtual void handleConnection( Connection_t& conn, Router_t& router ) = 0;
};
};   // namespace SandServer
