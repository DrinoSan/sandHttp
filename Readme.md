# Testing

https://github.com/codesenberg/bombardier

To test with curl and use a existing connection try:

- curl 127.0.0.1:8000 127.0.0.1:8000 --http0.9 -v

# Currenty Working on threadpool branch
Kevent is nice and i guess better but i want to implement a ThreadPool and also i want that the server works on Mac and Linux

# Current open Todos:

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
--------
- TODO: Need to add hasPrefix hasSuffix to HttpLib String library
- TODO: Need to to sanity checks on url
- TODO: Fix request parsing I forgot to check for content-type and then read until we have the full request.

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

- Implemented keep alive (threadpool)
- Implemented host header check which is mandatory
- Improved request parsing
- Added handling for Transfer-Encoding header value ----> not tested really
- Minimal string lib for prefix suffix removing reverse string

# Ressource

https://www.rfc-editor.org/rfc/rfc2616#section-4

# Dependencies
https://marzer.github.io/tomlplusplus/#mainpage-features
