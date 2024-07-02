# Testing
https://github.com/codesenberg/bombardier

To test with curl and use a existing connection try:
- curl 127.0.0.1:8000 127.0.0.1:8000 --http0.9 -v

# Current open Todos:
- TODO: need to convert all headers to lowercase befor adding
- ADD Headers to paths which are registerd!!!! MUST DO NEXT
  - Important for serving staticFiles content-type based of file extension
- Check TCP keep alive
- Big Brain: Split Server and SockerManager
  - SocketManager should take over all the socket stuff. The server has enough to do with Routing dispatching, Parsing of requests, session management and so on

# Future TODO:
- We need a way to identify user sessions.
  - The idea is to create a Session Class and a SessionManager for handling
  - The Session class itself would have a session identifier and this identifier would be passed as udata to kqueue :)


## Future
- Need to check errno 24 for accepting of new connections. Apparently we have to many files open with bombardier


## Implemented
Implemented readHTTPMessage method in SocketIOHandler to read full http message.
Using nodiscard to force me to use the return value.
The readHTTPMEssage will read untill we hit end delimiter \r\n\r\n



# Ressource
https://www.rfc-editor.org/rfc/rfc2616#section-4

