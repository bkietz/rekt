/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <rekt/utility.hpp>

namespace rekt
{
namespace
{

template <typename Symbol, typename Record>
constexpr auto
has_impl(type_constant<decltype(get(Symbol{}, std::declval<Record>()))> * = nullptr)
{
  return std::true_type{};
}

template <typename Symbol, typename Record>
constexpr auto has_impl(...)
{
  return std::false_type{};
}

template <typename Symbol, typename Record>
constexpr auto has = has_impl<Symbol, Record>(nullptr);

template <typename Symbol, typename Record, typename T = void>
using enable_if_has = std::enable_if_t<has<Symbol, Record>, T>;

///
/// symbol_set catches duplicates in a parameter pack of symbols.
/// If you see a build error here, you probably tried to define
/// a record's field for one symbol multiple times.
template <typename... Symbols>
struct symbol_set : private type_constant<Symbols>...
{
  constexpr symbol_set() = default;
  constexpr symbol_set(Symbols const &...) {}
};

template <>
struct symbol_set<>
{
  constexpr symbol_set() = default;
};

template <typename Record>
constexpr auto
has_all_impl(symbol_set<> const &)
{
  return std::true_type{};
}

template <typename Record, typename Symbol0, typename... Symbols>
constexpr auto
has_all_impl(symbol_set<Symbol0, Symbols...> const &)
{
  constexpr bool result = bool(has<Symbol0, Record>) && has_all_impl<Record>(symbol_set<Symbols...>{});
  return integer_constant<bool, result>{};
}

template <typename SymbolSet, typename Record>
constexpr auto has_all = has_all_impl<Record>(SymbolSet{});

} // namespace
} // namespace rekt
