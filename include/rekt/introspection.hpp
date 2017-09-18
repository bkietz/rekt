/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <rekt/record.hpp>

namespace rekt
{
namespace
{

struct properties : symbol<properties>
{
};

// field_enum
// get from an object with properties defined


template <typename Symbol, typename Record>
constexpr bool has_property_for = !std::is_same<Symbol, properties>()
  && has<properties, Record>
  && has<Symbol, field_type_for<properties, Record>>;

/*
template <typename Symbol, typename Record>
constexpr decltype(auto) get(Symbol const &, Record &&r,
  //std::enable_if_t<!std::is_same<properties, Symbol>()>* = nullptr,
  //enable_if_has<properties, Record&&>* = nullptr,
  //enable_if_has_2<properties, Symbol, Record&&>* = nullptr,
  //enable_if_has<Symbol, field_type_for<properties, Record&&>>* = nullptr)

  //std::enable_if_t<
  //  bool(!std::is_same<properties, Symbol>()) &&
  //  bool(has<properties, Record&&>) &&
  //  bool(has<Symbol, field_type_for<properties, Record&&>>)
  //>* = nullptr)

  //std::enable_if_t<
  //  bool(!std::is_same<properties, Symbol>()) &&
  //  bool(has<Symbol, field_type_for<properties, Record&&>>)
  //>* = nullptr)
  //std::enable_if_t<!std::is_same<properties, Symbol>()>* = nullptr)
{
  //return make_property_reference(get(Symbol{}, get(::rekt::properties{}, r)), std::forward<Record>(r));
  return make_property_reference(Symbol{}, ::rekt::properties{}, std::forward<Record>(r));
}

*/

template <class Class, typename Data>
constexpr decltype(auto) make_property_reference(Data std::decay_t<Class>::*mptr, Class &&o)
{
  return std::forward<Class&&>(o).*mptr;
}

template <typename Symbol, typename Record>
constexpr decltype(auto) get(Symbol const &, Record &&r,
  std::enable_if_t<has_property_for<Symbol, Record&&>>* = nullptr)
{
  //return make_property_reference(get(Symbol{}, get(::rekt::properties{}, r)), std::forward<Record>(r));
  return make_property_reference(Symbol{}, ::rekt::properties{}, std::forward<Record>(r));
}

} // namespace
} // namespace rekt
