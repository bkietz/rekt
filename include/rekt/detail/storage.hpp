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

template <typename Value, bool IsEmpty>
class storage;

template <typename Value>
class storage<Value, false>
{
public:
  using value_type = Value;

  template <typename... Args>
  constexpr storage(Args &&... args)
      : value_{ std::forward<Args>(args)... }
  {
  }

  constexpr decltype(auto) value() const
  {
    auto t = if_constexpr(std::is_reference<value_type>{},
		          type_c<value_type>,
                          type_c<value_type const &>);
    return static_cast_(t, value_);
  }

  constexpr decltype(auto) value()
  {
    auto t = if_constexpr(std::is_reference<value_type>{},
		          type_c<value_type>,
                          type_c<value_type &>);
    return static_cast_(t, value_);
  }

private:
  value_type value_;
};

template <typename Element>
class storage<Element[], false>
  : protected storage<Element*, false>
{
public:
  constexpr storage(Element *p)
    : storage<Element*, false>{ p }
  {
  }

  constexpr storage()
    : storage<Element*, false>{ nullptr }
  {
  }
};

template <typename EmptyValue>
class storage<EmptyValue, true>
    : private EmptyValue
{
public:
  using value_type = EmptyValue;

  template <typename... Args>
  constexpr storage(Args &&... args)
      : value_type{ std::forward<Args>(args)... }
  {
  }

  constexpr decltype(auto) value() const
  {
    return static_cast<value_type const &>(*this);
  }

  constexpr decltype(auto) value()
  {
    return static_cast<value_type &>(*this);
  }
};


} // namespace
} // namespace rekt
