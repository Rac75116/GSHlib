#pragma once
#include "TypeDef.hpp"
#include "Memory.hpp"
#include <string>

namespace gsh {

namespace internal {

    template<class CharT> using StrImpl = std::basic_string<CharT, std::char_traits<CharT>, Allocator<CharT>>;

    template<class CharT> using StrViewImpl = std::basic_string_view<CharT, std::char_traits<CharT>>;

}  // namespace internal

using Str = internal::StrImpl<ctype::c8>;
using Str8 = internal::StrImpl<ctype::utf8>;
using Str16 = internal::StrImpl<ctype::utf16>;
using Str32 = internal::StrImpl<ctype::utf32>;
using Strw = internal::StrImpl<ctype::wc>;

}  // namespace gsh
