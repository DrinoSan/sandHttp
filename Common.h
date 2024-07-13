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

// class StringContainer_t
//{
// public:
// StringContainer_t() = default;
// explicit StringContainer_t( std::vector<std::string> params_ )
//: params{ std::move( params_ ) }
//{
//}

//~StringContainer_t() = default;

// public:
// inline bool has( std::string_view param )
//{
// auto foundIt = std::find( params.begin(), params.end(), param );
// return foundIt == params.end();
//}

// inline void add( std::string& param ) { params.push_back( param ); }

// inline void add( const std::vector<std::string>& param )
//{
// params.insert( params.end(), param.begin(), param.end() );
//}

// inline void remove( std::string& param ) {}

//// Access the underlying vector's begin iterator
// inline auto begin() noexcept { return params.begin(); }
// inline auto begin() const noexcept { return params.begin(); }
// inline auto cbegin() const noexcept { return params.cbegin(); }

//// Access the underlying vector's end iterator
// inline auto end() noexcept { return params.end(); }
// inline auto end() const noexcept { return params.end(); }
// inline auto cend() const noexcept { return params.cend(); }

// private:
// std::vector<std::string> params;
//};
}   // namespace SandServer