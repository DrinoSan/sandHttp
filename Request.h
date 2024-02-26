#pragma once

// System HEADERS
#include <string>

namespace SandServer
{

enum class HttpMethod
{
    GET,
    POST,
    PUT,
    DELETE,
    UPDATE,
    HEAD,
    OPTIONS,
};

class Request_t
{
  public:
    HttpMethod method;
    // A own class for paths would be nice. I guess in future we have more often
    // to work with a path
    std::string requestTarget;
};

};   // namespace SandServer
