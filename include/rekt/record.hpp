/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <rekt/detail/array.hpp>
#include <rekt/detail/map_macro.hpp>
#include <rekt/record_traits.hpp>

namespace rekt
{
namespace
{

///
/// A field is one value associated with one symbol.
/// This class provides a dead simple concrete implementation
/// for use in defining/constructing records
///
/// A const record may have a field of mutable reference type.
template <typename Symbol, typename Value>
class field;

template <typename Value, bool IsEmpty>
class field_impl;

template <typename Value>
class field_impl<Value, false>
{
public:
  using value_type = Value;

  template <typename... Args>
  constexpr field_impl(Args &&... args)
      : value_{ std::forward<Args>(args)... }
  {
  }

  constexpr decltype(auto) value() const
  {
    return if_constexpr(std::is_reference<value_type>{},
                        static_cast<value_type>(value_),
                        static_cast<value_type const &>(value_));
  }

  decltype(auto) value()
  {
    return if_constexpr(std::is_reference<value_type>{},
                        static_cast<value_type>(value_),
                        static_cast<value_type &>(value_));
  }

private:
  value_type value_;
};

template <typename EmptyValue>
class field_impl<EmptyValue, true>
    : private EmptyValue
{
public:
  using value_type = EmptyValue;

  template <typename... Args>
  constexpr field_impl(Args &&... args)
      : value_type{ std::forward<Args>(args)... }
  {
  }

  constexpr decltype(auto) value() const
  {
    return static_cast<value_type const &>(*this);
  }

  decltype(auto) value()
  {
    return static_cast<value_type &>(*this);
  }

private:
  value_type value_;
};

template <typename Symbol, typename Value>
class field
    : private field_impl<Value, std::is_empty<Value>::value>
{
public:
  using value_type = Value;
  using storage_type = field_impl<Value, std::is_empty<Value>::value>;

  template <typename... Args>
  constexpr field(Args &&... args)
      : storage_type{ std::forward<Args>(args)... }
  {
  }

  constexpr decltype(auto) value() const
  {
    return storage_type::value();
  }

  decltype(auto) value()
  {
    return storage_type::value();
  }
};

template <typename Symbol, typename Value>
constexpr decltype(auto) get(Symbol const &, field<Symbol, Value> &&rref)
{
  return std::move(rref.value());
}

template <typename Symbol, typename Value>
constexpr decltype(auto) get(Symbol const &, field<Symbol, Value> const &clref)
{
  return clref.value();
}

template <typename Symbol, typename Value>
constexpr decltype(auto) get(Symbol const &, field<Symbol, Value> &lref)
{
  return lref.value();
}

///
/// make_field will always produce a field with reference
/// type, either rvalue or lvalue.
template <typename Symbol, typename Value>
constexpr auto make_field(Symbol const &, Value &&v)
{
  return field<Symbol, decltype(std::forward<Value &&>(v))>(std::forward<Value &&>(v));
}

///
/// assign field values in one record to those in another for each
/// field associated with the symbols in a symbol_set
template <typename Target, typename Source, typename... Symbols>
decltype(auto) assign_fields(Target &&t, Source &&s, symbol_set<Symbols...> const &)
{
  static_assert(has_all<symbol_set<Symbols...>, Target &&>, "fields were not defined on target record for all symbols in assign_fields() call");
  static_assert(has_all<symbol_set<Symbols...>, Source &&>, "fields were not defined on source record for all symbols in assign_fields() call");
  symbol_set<Symbols...>{ (get(Symbols{}, std::forward<Target>(t)) = get(Symbols{}, std::forward<Source>(s)), Symbols{})... };
  return std::forward<Target>(t);
}

///
/// simple implementation of a record with several stored fields
template <typename... Fields>
class record;

template <typename... Symbol, typename... Value>
class record<field<Symbol, Value>...>
    : private symbol_set<Symbol...>,
      public field<Symbol, Value>...
{
public:
  template <typename... Args>
  record(Args &&... args)
      : field<Symbol, Value>{ std::forward<Args>(args) }...
  {
  }

  ///
  /// defined to enable std::tie style assignment
  template <typename OtherRecord>
  record &operator=(OtherRecord &&other)
  {
    return assign_fields(*this, std::forward<OtherRecord>(other), symbol_set<Symbol...>{});
  }
};

///
/// make_record decays field values, moving from rvalue references
/// and copying everything else
template <typename... Symbol, typename... Value>
constexpr auto make_record(field<Symbol, Value> &&... fields)
{
  return record<field<Symbol, std::decay_t<Value>>...>{ fields.value()... };
}

///
/// forward_as_record leaves references undecayed
template <typename... Symbol, typename... Value>
constexpr auto forward_as_record(field<Symbol, Value> &&... fields)
{
  return record<field<Symbol, Value>...>{ std::move(fields)... };
}

///
/// CRTP helper for augmenting tag types used as symbols with some
/// nifty, Haskell-style operator overloads
template <typename Symbol>
struct symbol
{
  template <typename Value>
  constexpr auto operator=(Value &&value) const
  {
    return make_field(Symbol{}, std::forward<Value>(value));
  }

  template <typename Value>
  constexpr auto operator&=(Value &value) const
  {
    return make_field(Symbol{}, std::ref(value));
  }

  template <typename Value, std::size_t N>
  constexpr auto operator=(Value const (&value)[N]) const
  {
    return field<Symbol, std::array<Value, N>>(make_array(value));
  }

  template <typename Record>
  constexpr decltype(auto) operator()(Record &&r) const
  {
    return get(Symbol{}, std::forward<Record>(r));
  }
};

template <typename L, typename R>
constexpr auto operator==(symbol<L> const &, symbol<R> const &)
{
  return std::is_same<L, R>{};
}

template <typename L, typename R>
constexpr auto operator!=(symbol<L> const &l, symbol<R> const &r)
{
  return integer_c<bool, !(l == r)>;
}

///
/// Not Haskell (REBOL, anybody?) but allows concise field access
/// which resembles `rec.field_name` if you squint.
template <typename Record, typename Symbol>
constexpr decltype(auto) operator/(Record &&r, symbol<Symbol> const &)
{
  return get(Symbol{}, std::forward<Record>(r));
}

///
/// In case Symbol was not defined using the symbol CRTP base, these
/// helpers provide cute access to operator=
///
///     let<x>  = 32  // make_field(x{}, 32)
///     let_(y) = 43  // make_field(y,   43)
template <typename Symbol>
constexpr auto let = symbol<Symbol>{};

template <typename Symbol>
constexpr auto let_(Symbol const &)
{
  return symbol<Symbol>{};
}

template <typename Symbol>
constexpr auto let_(symbol<Symbol> const &)
{
  return symbol<Symbol>{};
}

///
/// Yet another helper to sweeten make_field
///
///     32 - as<x>   // make_field(x{}, 32)
///     43 - as_(y)  // make_field(y,   43)
template <typename Symbol>
struct as_t
{
};

template <typename Value, typename Symbol>
constexpr auto operator-(Value &&value, as_t<Symbol> const &)
{
  return make_field(Symbol{}, std::forward<Value>(value));
}

template <typename Symbol>
constexpr auto as = as_t<Symbol>{};

template <typename Symbol>
constexpr auto as_(Symbol const &)
{
  return as_t<Symbol>{};
}

template <typename Symbol>
constexpr auto as_(symbol<Symbol> const &)
{
  return as_t<Symbol>{};
}

///
/// Construct a record using only the fields associated
/// with the given symbol set.
template <typename... Symbols, typename Record>
constexpr auto take(Record &&r, symbol_set<Symbols...> const & = {})
{
  return make_record(make_field(Symbols{}, get(Symbols{}, std::forward<Record>(r)))...);
}

template <typename... Symbols, typename Record>
constexpr auto take(Record &&r, Symbols const &...)
{
  return make_record(make_field(Symbols{}, get(Symbols{}, std::forward<Record>(r)))...);
}

///
/// Construct a record by applying a function to each field
/// associated with the symbols from a the given symbol set.
/// The arguments passed to the function are a symbol from
/// the symbol set and the value of the mapped record for that
/// symbol.
template <typename... Symbols, typename Record, typename Function>
constexpr auto map(Record &&r, Function &&f, symbol_set<Symbols...> const & = {})
{
  return make_record(make_field(Symbols{}, f(Symbols{}, get(Symbols{}, std::forward<Record>(r))))...);
}

///
/// std::get style alias
template <typename Symbol, typename Record>
constexpr decltype(auto) get(Record &&r)
{
  return get(Symbol{}, std::forward<Record>(r));
}

///
/// Create a non-owning merged view of several records.
/// Field access for a symbol on the view is deferred to the
/// first of the records which defines a field for that symbol.
template <typename... RecordRefs>
class merged_records;

template <typename... Records>
constexpr merged_records<Records &&...> merge(Records &&... rs);

template <typename... RecordRefs>
class merged_records
{
public:
  constexpr merged_records(RecordRefs... refs)
      : refs_{ static_cast<RecordRefs>(refs)... }
  {
  }

private:
  template <typename Symbol>
  static constexpr std::size_t index_()
  {
    return 0;
  }
  template <typename Symbol, typename Head, typename... Tail>
  static constexpr std::size_t index_()
  {
    return has<Symbol, Head>
        ? 0
        : index_<Symbol, Tail...>() + 1;
  }

  template <typename Symbol>
  friend constexpr decltype(auto) get(Symbol const &s, merged_records const &m,
                                      std::enable_if_t<index_<Symbol, RecordRefs...>() < sizeof...(RecordRefs)> * = nullptr)
  {
    return get(s, std::get<index_<Symbol, RecordRefs...>()>(m.refs_));
  }

  std::tuple<RecordRefs...> refs_;
};

template <typename... Records>
constexpr merged_records<Records &&...> merge(Records &&... rs)
{
  return merged_records<Records &&...>{ std::forward<Records>(rs)... };
}

///
/// Operator overloads providing easy access to merge()
/// in the common case where you just want to override or
/// give a default value for a couple fields for one expression.

///
/// get(x, r ^ (x = 3)) == 3 // override r's field for x, if any
template <typename Record, typename Symbol, typename Value>
constexpr auto operator^(Record &&r, field<Symbol, Value> &&aug)
{
  return merge(std::move(aug), std::forward<Record>(r));
}

template <typename Record, typename... Symbols, typename... Values>
constexpr auto operator^(Record &&r, record<field<Symbols, Values>...> &&aug)
{
  return merge(std::move(aug), std::forward<Record>(r));
}

///
/// get(x, r & (x = 3)) == get(x, r) // if r has a field for x
///                     == 3         // if not
template <typename Record, typename Symbol, typename Value>
constexpr auto operator&(Record &&r, field<Symbol, Value> &&aug)
{
  return merge(std::forward<Record>(r), std::move(aug));
}

template <typename Record, typename... Symbols, typename... Values>
constexpr auto operator&(Record &&r, record<field<Symbols, Values>...> &&aug)
{
  return merge(std::forward<Record>(r), std::move(aug));
}

} // namespace
} // namespace rekt
