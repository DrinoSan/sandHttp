#pragma once

// System Headers
#include <string>
#include <vector>

// Project Headers
#include "Common.h"

namespace SandServer
{
class Url_t
{
  public:
    Url_t();
    ~Url_t();

  public:
    StringContainer_t query;
    StringContainer_t urlSegments;
};
}   // namespace SandServer