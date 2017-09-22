/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <utility>

namespace rekt
{
namespace
{

template <typename Int, Int i>
using integer_constant = std::integral_constant<Int, i>;

template <typename Int, Int i>
constexpr auto integer_c = integer_constant<Int, i>{};

using std::integer_sequence;

using std::make_integer_sequence;

template <std::size_t i>
using index_constant = integer_constant<std::size_t, i>;

template <std::size_t i>
constexpr auto index_c = index_constant<i>{};

using std::index_sequence;

using std::make_index_sequence;

using std::index_sequence_for;

template <typename T>
struct type_constant
{
  using type = T;
  constexpr decltype(auto) declval() const noexcept
  {
    return std::declval<type>();
  }
};

template <typename T>
constexpr auto type_c = type_constant<T>{};

template <typename... Ts>
struct type_sequence
{
};

template <typename T, T &ref>
struct reference_constant
{
  using value_type = T &;
  static constexpr value_type value = ref;
  using type = reference_constant;
  constexpr operator value_type() const noexcept { return value; }
};

template <typename T, T &ref>
constexpr auto reference_c = reference_constant<T, ref>{};

template <typename C, typename T, T C::*ptr>
struct member_pointer_constant
{
  using value_type = T C::*;
  static constexpr value_type value = ptr;
  using type = member_pointer_constant;
  constexpr operator value_type() const noexcept { return value; }
};

template <typename C, typename T, T C::*ptr>
constexpr auto member_pointer_c = member_pointer_constant<C, T, ptr>{};

template <typename Then, typename Else>
constexpr decltype(auto) if_constexpr(std::true_type const &, Then &&t, Else &&)
{
  return std::forward<Then>(t);
}

template <typename Then, typename Else>
constexpr decltype(auto) if_constexpr(std::false_type const &, Then &&, Else &&e)
{
  return std::forward<Else>(e);
}

template <typename To, typename From>
constexpr To static_cast_(type_constant<To>, From &&f)
{
  return static_cast<To>(std::forward<From>(f));
}

} // namespace
} // namespace rekt
