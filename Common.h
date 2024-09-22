#pragma once

// System Headers
#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace SandServer
{
using StringContainer_t = std::vector<std::string>;
template <typename T, typename U>
typename std::enable_if<( std::is_same<T, std::vector<std::string>>::value &&
                          std::is_same<U, std::string>::value ) ||
                            ( std::is_same<T, std::vector<int32_t>>::value &&
                              std::is_same<U, int32_t>::value ),
                        bool>::type
has( const T& params, const U& param )
{
    auto foundIt = std::find( params.begin(), params.end(), param );
    return foundIt != params.end();
}
}   // namespace SandServer
