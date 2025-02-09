#pragma once

class WebSocketHandler : public ProtocolHandler
{
 public:
   void handleRequest( int clientSocket ) override;

 private:
   void        performHandshake( int clientSocket );
   std::string readFrame( int clientSocket );
   void        sendFrame( int clientSocket, const std::string& message );
};
