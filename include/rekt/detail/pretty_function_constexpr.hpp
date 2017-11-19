/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <rekt/detail/pretty_function_macro.hpp>
#include <rekt/utility.hpp>

namespace rekt
{
namespace
{
namespace pretty_function
{

template <typename T>
constexpr char const *raw()
{
  return REKT_PRETTY_FUNCTION;
}

template <typename T>
constexpr std::size_t raw_sizeof()
{
  // using sizeof to get the length of type names requires correction
  // but means that we don't need recursive constexpr string processing
  return sizeof(REKT_PRETTY_FUNCTION);
}

constexpr bool starts_with(char const *haystack, char const *needle)
{
  if (needle[0] == '\0') return true;
  return haystack[0] == needle[0] && starts_with(haystack + 1, needle + 1);
}

constexpr std::size_t search(char const *haystack, char const *needle)
{
  if (haystack[0] == '\0') return 0;
  if (starts_with(haystack, needle)) return 0;
  return search(haystack + 1, needle) + 1;
}

constexpr std::size_t prefix_length = search(raw<void>(), "void");

template <typename T>
struct type_name
{
  static constexpr char const *decorated_begin = raw<T>() + prefix_length;
  
  static constexpr std::size_t decoration_length = class_names_bare || !std::is_class<T>::value
    ? 0
    : decorated_begin[0] == 's'
      ? 7   // starts_with(decorated_begin, "struct ")
      : 6; // starts_with(decorated_begin, "class ")

  static constexpr char const *begin = decorated_begin + decoration_length;

  // raw_sizeof<T>() - raw_sizeof<void>() == decoration_length + (length of T's name) - strlen("void")
  // (length of T's name) == raw_sizeof<T>() - raw_sizeof<void>() + strlen("void") - decoration_length
  static constexpr std::size_t length = raw_sizeof<T>() - raw_sizeof<void>() + 4 - decoration_length;

  static constexpr std::array<char, length> value = make_array(begin, index_c<length>);
};

struct crib {};
constexpr struct crib crib = {};

struct anonymous_scope
{
  // (crib's name) == "rekt::" + (anonymous scope) + "pretty_function::crib"
  static constexpr char const *begin = type_name<struct crib>::begin + sizeof("rekt::") - 1;

  static constexpr std::size_t length = search(begin, "pretty_function::crib");

  static constexpr std::array<char, length> value = make_array(begin, index_c<length>);
};

} // namespace pretty_function
} // namespace
} // namespace rekt
