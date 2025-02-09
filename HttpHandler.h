#pragma once

// Project Headers
#include "Connection.h"
#include "HttpMessage.h"
#include "ProtocolHandler.h"
#include "Router.h"
#include "SocketIOHandler.h"

namespace SandServer
{
class HttpHandler_t : public ProtocolHandler_t
{
 public:
   void handleConnection( Connection_t& conn, Router_t& router ) override;

 private:
   std::pair<HTTPResponse_t, bool>
   generateResponse( HandlerFunc& handler, HTTPRequest_t& httpRequest );

   SocketIOHandler_t socketIOHandler;
};
};   // namespace SandServer
