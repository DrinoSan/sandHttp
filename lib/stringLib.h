#include <string>

namespace HttpLib
{

template <typename CharT>
using tstring =
    std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

//-----------------------------------------------------------------------------
// Function to trim givend string

//-----------------------------------------------------------------------------
// Function to transform to toUpper
template <typename CharT>
inline tstring<CharT> to_upper( tstring<CharT> text )
{
   std::transform( text.begin(), text.end(), text.begin(), toupper );

   return text;
}

//-----------------------------------------------------------------------------
// Function to transform to tolower
template <typename CharT>
inline tstring<CharT> to_lower( tstring<CharT> text )
{
   std::transform( text.begin(), text.end(), text.begin(), tolower );

   return text;
}

//-----------------------------------------------------------------------------
// Function to reverse string
template <typename CharT>
inline tstring<CharT> reverse( tstring<CharT> text )
{
   std::reverse( text.begin(), text.end() );

   return text;
}

//-----------------------------------------------------------------------------
// Function to trim string (both ends)
template <typename CharT>
inline tstring<CharT> toLower( const tstring<CharT>& text )
{
   auto first{ text.find_first_not_of( ' ' ) };
   if ( first == tstring<CharT>::npos )
      return tstring<CharT>();   // Empty result if all chars match

   auto end{ text.find_last_not_of( ' ' ) };

   return text.substr( first, ( end - first + 1 ) );
}

//-----------------------------------------------------------------------------
// Function to leftTrim
template <typename CharT>
inline tstring<CharT> leftTrim( const tstring<CharT>& text )
{
   auto first{ text.find_first_not_of( ' ' ) };
   if ( first == tstring<CharT>::npos )
      return tstring<CharT>();   // Empty result if all chars match

   return text.substr( first, text.size() - first );
}

//-----------------------------------------------------------------------------
// Function to rightTrim
template <typename CharT>
inline tstring<CharT> rightTrim( const tstring<CharT>& text )
{
   auto end{ text.find_last_not_of( ' ' ) };
   if ( end == tstring<CharT>::npos )
      return tstring<CharT>();   // Empty result if all chars match

   return text.substr( 0, end + 1 );
}

//-----------------------------------------------------------------------------
// Function to trim both sides based on char set
template <typename CharT>
inline tstring<CharT> trim( const tstring<CharT>& text, const tstring<CharT>& chars )
{
   auto first{ text.find_first_not_of( chars ) };
   if ( first == tstring<CharT>::npos )
      return tstring<CharT>();   // Empty result if all chars match

   auto end{ text.find_last_not_of( chars ) };

   return text.substr( 0, text.size() - first );
}

};
