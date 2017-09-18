/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>
#include <rekt/utility.hpp>

namespace rekt
{
namespace
{

template <typename Element, std::size_t... I>
constexpr std::array<Element, sizeof...(I)> make_array(Element const *ptr, index_sequence<I...> const &)
{
  return std::array<Element, sizeof...(I)>{ ptr[I]... };
}

template <typename Element, std::size_t N>
constexpr std::array<Element, N> make_array(Element const *ptr, index_constant<N> const &)
{
  return make_array(ptr, make_index_sequence<N>{});
}

template <typename Element, std::size_t N>
constexpr std::array<Element, N> make_array(Element const (&arr)[N])
{
  return make_array(arr, make_index_sequence<N>{});
}

} // namespace
} // namespace rekt
