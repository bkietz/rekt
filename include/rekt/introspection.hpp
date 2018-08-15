/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <rekt/record.hpp>
#include <rekt/detail/pretty_function.hpp>
#include <string>

namespace rekt
{
namespace
{

template <typename Symbol>
std::string nameof(Symbol const & = {})
{
  constexpr auto const &n = pretty_function::type_name<Symbol>::value;
  return std::string{ n.begin(), n.end() };
}

///
/// get(sym, get(rekt::properties{}, rec)) should be a value from which a
///   property accessor for `sym` can be constructed
/// don't use this for a field in rekt::record or things will start breaking
///   TODO assert, don't comment
struct properties : symbol<properties>
{
};

///
/// field_enum can be defined to provide a record mapping symbols to index_constants
///   which makes a declared, ordered listing of symbols for a type.
///   field_enum is already defined for rekt::record and any type which has rekt::properties
struct field_enum : symbol<struct field_enum>
{
  template <typename... Symbols, std::size_t... I>
  constexpr auto make(symbol_set<Symbols...> const&, index_sequence<I...> const&) const
  {
    return make_record(make_field(Symbols{}, index_c<I>)...);
  }

  template <typename... Symbols>
  constexpr auto make(Symbols const&...) const
  {
    return make(symbol_set<Symbols...>{}, index_sequence_for<Symbols...>{});
  }

  template <typename Record, typename Function>
  constexpr auto map(Record &&r, Function &&f) const
  {
    return rekt::map(get(field_enum{}, r), std::forward<Function>(f));
  }
};

constexpr struct field_enum field_enum = {};

template <typename... Symbol, typename... Value>
constexpr auto get(struct field_enum const&, record<field<Symbol, Value>...> const&)
{
  return field_enum.make(symbol_set<Symbol...>{}, index_sequence_for<Symbol...>{});
}



///
/// superjank type trait to help visual studio out with enabling fields for one symbol
/// based on the presence of another
template <typename Symbol, typename Record>
constexpr bool has_property_for(std::false_type is_properties, std::true_type has_properties, std::true_type has_symbol)
{
  return true;
}

template <typename Symbol, typename Record>
constexpr bool has_property_for(bool, ...)
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

///
/// simplest property case: data member pointer
template <class Class, typename Data>
constexpr decltype(auto) make_property_reference(Data std::decay_t<Class>::*mptr, Class &&o)
{
  return std::forward<Class>(o).*mptr;
}

template <typename Ptr>
struct wrapped_member_ptr_getter
{
  Ptr ptr;

  template <typename Class>
  constexpr decltype(auto) operator()(Class &&o) const
  {
    return (std::forward<Class>(o).*ptr)();
  }
};

template <typename Data, typename RecordRef>
constexpr auto wrap_getter(Data g(RecordRef))
{
  return g;
}

template <class Class, typename Data>
constexpr auto wrap_getter(Data (Class::*g)())
{
  return wrapped_member_ptr_getter<Data(Class::*)()>{ g };
}

template <typename Ptr>
struct wrapped_member_ptr_setter
{
  Ptr ptr;

  template <typename Class, typename Value>
  void operator()(Class &&o, Value &&v) const
  {
    (std::forward<Class>(o).*ptr)(std::forward<Value>(v));
  }
};

template <typename Return, typename RecordRef, typename Data>
constexpr auto wrap_setter(Return s(RecordRef, Data))
{
  return s;
}

template <class Class, typename Data, typename Return>
constexpr auto wrap_setter(Return (Class::*s)(Data))
{
  return wrapped_member_ptr_setter<Return (Class::*)(Data)>{ s };
}

template <typename Getter, typename Setter>
constexpr auto get_set(Getter &&g, Setter &&s)
{
  return std::make_pair(
      wrap_getter(std::forward<Getter &&>(g)),
      wrap_setter(std::forward<Setter &&>(s)));
}

template <typename Getter, typename Setter, typename RecordRef, typename Data>
struct getter_setter_property_reference
{
  std::pair<Getter, Setter> gs_;
  RecordRef ref_;

  constexpr decltype(auto) cast() const
  {
    return gs_.first(static_cast<RecordRef>(ref_));
  }

  constexpr operator Data() const
  {
    return cast();
  }

  template <typename Value>
  RecordRef &operator=(Value &&v) const
  {
    gs_.second(static_cast<RecordRef>(ref_), std::forward<Value>(v));
    return ref_;
  }
};

template <typename Getter, typename Setter, typename Record>
constexpr auto make_property_reference(std::pair<Getter, Setter> gs, Record &&r)
{
  using data_type = decltype(gs.first(std::forward<Record>(r)));
  return getter_setter_property_reference<Getter, Setter, Record, data_type>{ gs, std::forward<Record>(r) };
}

template <typename Getter, typename RecordRef, typename Data>
struct getter_property_reference
{
  Getter g_;
  RecordRef ref_;

  constexpr decltype(auto) cast() const
  {
    return g_(static_cast<RecordRef>(ref_));
  }

  constexpr operator Data() const
  {
    return cast();
  }
};

template <typename Getter, typename Record>
constexpr auto make_property_reference(Getter &&g, Record &&r)
{
  using data_type = decltype(g(std::forward<Record>(r)));
  return getter_property_reference<Getter, Record, data_type>{ g, std::forward<Record>(r) };
}

///
/// NB: in general, it's a bad idea to define a symbol based on the presence of another
template <typename Symbol, typename Record>
constexpr decltype(auto) get(Symbol const &, Record &&r,
                             std::enable_if_t<has_property_for<Symbol, Record &&>()> * = nullptr)
{
  return make_property_reference(get(Symbol{}, get(::rekt::properties{}, r)), std::forward<Record>(r));
}

template <typename Record>
constexpr auto get(struct field_enum const&, Record &&r,
  enable_if_has<properties, Record&&> * = nullptr)
{
  return field_enum(get(properties{}, std::forward<Record>(r)));
}

template <typename AssociativeContainer, typename Record>
void unpack(AssociativeContainer const &container, Record *rec)
{
  // TODO allow custom conversion
  // FIXME handle readonly fields
  // FIXME report key/values which don't correspond to a field
  field_enum.map(*rec, [rec, &container](auto const &sym, std::size_t)
  {
    auto it = container.find(nameof(sym));
    // FIXME handle missing fields
    get(sym, *rec) = container[nameof(sym)];
    return true;
  });
}

template <typename Record, typename AssociativeContainer>
void pack(Record &&rec, AssociativeContainer *container)
{
  // TODO allow custom conversion
  // FIXME handle writeonly fields
  field_enum.map(rec, [&rec, container](auto const &sym, std::size_t index)
  {
    container->emplace(nameof(sym), get(sym, std::forward<Record>(rec)));
    return true;
  });
}

} // namespace
} // namespace rekt
