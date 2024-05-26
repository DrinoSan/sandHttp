# Testing
https://github.com/codesenberg/bombardier

To test with curl and use a existing connection try:
- curl 127.0.0.1:8000 127.0.0.1:8000 --http0.9 -v

# Current open Todos:
- Create sigaction to exit threads in a normal way and also listendAndAccept function
- Check TCP keep alive
- Would be nice to have a logging library like https://github.com/gabime/spdlog
  - [x] Added
- Work on Request class
  - Started work. First I deleted the request file and we now have a HttpMessage file with 3 classes
    - HttpMessage, HttpRequest, HttpResponse
- Big Brain: Split Server and SockerManager
  - SocketManager should take over all the socket stuff. The server has enough to do with Routing dispatching, Parsing of requests, session management and so on
    - Started work on that with SocketIOHandler.h
  - Implement function to write to socket in server
    - Is currently in SocketIOHandler
      - We need to split readRawSocket and readHttpMessage, I want to split the low level stuff from higher level
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



# Ressource
https://www.rfc-editor.org/rfc/rfc2616#section-4

