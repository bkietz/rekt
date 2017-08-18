/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifdef _MSC_VER
#pragma message("As of this writing, Visual Studio does not support the string literal operator template")
#endif // _MSC_VER

#include <rekt/record.hpp>

namespace rekt
{
namespace
{
inline namespace string_literals
{

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"

template <typename Char, Char... c>
constexpr auto operator""_()
{
  return symbol_c<integer_sequence<Char, c...>>;
}

#pragma clang diagnostic pop

} // namespace string_literals
} // namespace
} // namespace rekt
