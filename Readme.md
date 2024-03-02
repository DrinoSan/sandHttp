# Testing
https://github.com/codesenberg/bombardier

# Current open Todos:
- Would be nice to have a logging library like https://github.com/gabime/spdlog
- Work on Request class
- Implement function to write to socket in server_t
- Big Brain: Split Server_t and SockerManager
  - SocketManager should take over all the socket stuff. The server has enough to do with Routing dispatching, Parsing of requests, session management and so on
  - Need to take care of synchronization then ????

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

