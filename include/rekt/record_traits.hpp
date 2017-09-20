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

template <typename Symbol>
struct has_impl
{
  template <typename Record>
  static constexpr auto
  test(type_constant<Record> const &, type_constant<decltype(get(Symbol{}, std::declval<Record>()))> * = nullptr)
  {
    return std::true_type{};
  }

  template <typename Record>
  static constexpr auto test(type_constant<Record> const &, ...)
  {
    return std::false_type{};
  }

  static constexpr auto
  test(type_constant<has_impl<Symbol>> const &, ...)
  {
    return has_impl<Symbol>();
  }

  template <typename Record>
  constexpr auto operator()(Record &&) const
  {
    return test<Symbol>(type_c<Record &&>, nullptr);
  }
};

template <typename Symbol, typename Record = has_impl<Symbol>>
constexpr auto has = has_impl<Symbol>::test(type_c<Record>, nullptr);

template <typename Symbol>
constexpr auto has_(Symbol const &)
{
  return has<Symbol>;
}

template <typename Symbol, typename Record>
constexpr auto has_(Symbol const &, Record &&)
{
  return has<Symbol, Record &&>;
}

template <typename Symbol, typename Record, typename T = void>
using enable_if_has = std::enable_if_t<has<Symbol, Record>, T>;

template <typename Symbol, typename Record, typename T = void>
using disable_if_has = std::enable_if_t<!has<Symbol, Record>, T>;

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

template <typename Symbol, typename Record>
using field_type_for = decltype(get(Symbol{}, type_c<Record>.declval()));

} // namespace
} // namespace rekt
