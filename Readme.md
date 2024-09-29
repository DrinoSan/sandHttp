# Testing

https://github.com/codesenberg/bombardier

To test with curl and use a existing connection try:

- curl 127.0.0.1:8000 127.0.0.1:8000 --http0.9 -v

# Current open Todos:

- TODO fix bug with sighandler
- TODO: create config to read from
  - Make sure the values in config are not bigger than K_NUM_WORKER_MAX and so on
  - This needs to be checked because at class level i create the 2d arrays and for that i need compile time known integer values and cant use the values from the config. Therefore i create them with some MAX value but onlu use the values from the config
- TODO: need to convert all headers to lowercase befor adding
- TODO: Query in route
  - https://example.org/?a=1&a=2&b=&=3&&&& as example for a url with query ? starts the query & is a delimiter for multiple parameters
- TODO: Middleware support
- TODO: Body parsing( JSON, XML...)
- TODO: File upload for users
- TODO: Caching or static content and dynamic responses
- TODO: Security
  - IP whitelisting/blacklisting
  - CSRF protection
  - SQL injection prevention.
- TODO: WebSockets
- TODO: Server-Sent Events (SSE)

- Sockets - Streaming
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

# Dependencies
https://marzer.github.io/tomlplusplus/#mainpage-features
