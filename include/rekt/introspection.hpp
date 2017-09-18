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

///
/// superjank type trait to help visual studio out with enabling fields for one symbol
/// based on the presence of another
template <typename Symbol, typename Record>
constexpr bool has_property_for(std::false_type is_properties, std::true_type has_properties, std::true_type has_symbol)
{
  return true;
}

template <typename Symbol, typename Record>
constexpr bool has_property_for(...)
{
  return false;
}

template <typename Symbol, typename Record>
constexpr bool has_property_for(std::false_type is_properties, std::true_type has_properties)
{
  auto has_symbol = has<Symbol, field_type_for<properties, Record>>;
  return has_property_for<Symbol, Record>(is_properties, has_properties, has_symbol);
}

template <typename Symbol, typename Record>
constexpr bool has_property_for(std::false_type is_properties)
{
  auto has_properties = has<properties, Record>;
  return has_property_for<Symbol, Record>(is_properties, has_properties);
}

template <typename Symbol, typename Record>
constexpr bool has_property_for()
{
  auto is_properties = std::is_same<Symbol, properties>();
  return has_property_for<Symbol, Record>(is_properties);
}

template <class Class, typename Data>
constexpr decltype(auto) make_property_reference(Data std::decay_t<Class>::*mptr, Class &&o)
{
  return std::forward<Class&&>(o).*mptr;
}

///
/// NB: in general, it's a bad idea to define a symbol based on the presence of another
template <typename Symbol, typename Record>
constexpr decltype(auto) get(Symbol const &, Record &&r,
  std::enable_if_t<has_property_for<Symbol, Record&&>()>* = nullptr)
{
  return make_property_reference(get(Symbol{}, get(::rekt::properties{}, r)), std::forward<Record>(r));
}

} // namespace
} // namespace rekt
