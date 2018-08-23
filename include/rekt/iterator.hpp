/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>
#include <iterator>
#include <rekt/record.hpp>

namespace rekt
{
namespace
{

///
/// NB: all iterators zipped herein *must* provide valid traits accessible via std::iterator_traits
/// NB: Also they must be EqualityComparable
struct zip
{
  template <typename... Symbols, typename... ContainerRefs>
  constexpr auto operator()(field<Symbols, ContainerRefs>... container_ref_fields) const;

  template <typename ContainerRecord>
  class range;

  // catch-all tag for non-standard iterator categories
  struct iterator_tag
  {
  };

  static constexpr auto iterator_category_ordering = make_record(
      // satisfies Iterator, but not InputIterator or OutputIterator
      make_field(iterator_tag{}, index_c<0>),

      // InputIterator or OutputIterator are both single pass
      make_field(std::output_iterator_tag{}, index_c<1>),
      make_field(std::input_iterator_tag{}, index_c<1>),

      // the rest are a more straightforward heirarchy
      make_field(std::forward_iterator_tag{}, index_c<2>),
      make_field(std::bidirectional_iterator_tag{}, index_c<3>),
      make_field(std::random_access_iterator_tag{}, index_c<4>));

  template <typename IteratorCategory>
  static constexpr auto iterator_category_ordering_v = get(IteratorCategory{}, iterator_category_ordering);

  template <typename IteratorOrIteratorRecord>
  struct iterator_category;

  template <typename IteratorCategory, typename IteratorRecord>
  class iterator;

  template <typename IteratorRecord>
  struct reference;

  struct begin
  {
    template <typename Symbol, typename Container>
    constexpr auto operator()(Symbol const &, Container &&c) const
    {
      return std::begin(std::forward<Container>(c));
    }
  };

  struct end
  {
    template <typename Symbol, typename Container>
    constexpr auto operator()(Symbol const &, Container &&c) const
    {
      return std::end(std::forward<Container>(c));
    }
  };
};

template <typename... Symbols, typename... Containers>
class zip::range<record<field<Symbols, Containers>...>>
    : public record<field<Symbols, Containers>...>
{
public:
  using container_record = record<field<Symbols, Containers>...>;

  using iterator_record = decltype(map(std::declval<container_record &>(), zip::begin{}));
  static_assert(std::is_same<decltype(map(std::declval<container_record &>(), zip::end{})), iterator_record>::value,
                "end and begin must return same type");
  using iterator_category = typename zip::iterator_category<iterator_record>::type;
  using iterator = zip::iterator<iterator_category, iterator_record>;

  range(rekt::field<Symbols, Containers>... container_ref_fields)
      : container_record{ std::move(container_ref_fields)... }
  {
  }

  iterator begin()
  {
    return iterator{ map(*this, zip::begin{}) };
  }

  iterator end()
  {
    return iterator{ map(*this, zip::end{}) };
  }
};

template <typename Iterator>
struct zip::iterator_category
{
  static constexpr zip::iterator_tag impl(...);

  template <typename Traits>
  static constexpr typename Traits::iterator_category impl(Traits);

  using type = decltype(impl(std::iterator_traits<Iterator>{}));
};

template <>
struct zip::iterator_category<record<>>
{
  using type = zip::iterator_tag;
};

template <typename... Symbols, typename... Iterators>
struct zip::iterator_category<record<field<Symbols, Iterators>...>>
{
  using category_record = record<field<Symbols, typename iterator_category<Iterators>::type>...>;

  static constexpr zip::iterator_tag impl(index_constant<0>);

  static constexpr auto impl(index_constant<1>)
  {
    // at least one input or output iterator is present
    constexpr bool input = std::max({ (std::is_same<decltype(get(Symbols{}, category_record{})), std::input_iterator_tag>::value)... });
    constexpr bool output = std::max({ (std::is_same<decltype(get(Symbols{}, category_record{})), std::output_iterator_tag>::value)... });

    static_assert(input || output, "how did this happen");

    return std::conditional_t<input,
                              std::conditional_t<output,
                                                 iterator_tag,             // input && output- downgrade to Iterator
                                                 std::input_iterator_tag>, // input && !output
                              std::output_iterator_tag>{};                 // !input && output
  }

  template <std::size_t N>
  static constexpr auto impl(index_constant<N>)
  {
    auto reverse = make_record(
        make_field(index_c<2>, std::forward_iterator_tag{}),
        make_field(index_c<3>, std::bidirectional_iterator_tag{}),
        make_field(index_c<4>, std::random_access_iterator_tag{}));

    return get(index_c<N>, reverse);
  }

  constexpr static auto minimum_iterator_category_ord = index_c<std::min({ iterator_category_ordering_v<typename iterator_category<Iterators>::type>... })>;

  using type = decltype(impl(minimum_iterator_category_ord));
};

template <typename... Symbols, typename... Iterators>
class zip::reference<record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, typename std::iterator_traits<Iterators>::reference>...>
{
public:
  using reference_record = record<field<Symbols, typename std::iterator_traits<Iterators>::reference>...>;
  using reference_record::operator=;

  template <typename Category>
  reference(zip::iterator<Category, record<field<Symbols, Iterators>...>> it)
      : reference_record{ *get(Symbols{}, it)... }
  {
  }

  reference(reference const &) = delete;
  reference &operator=(reference const &) = delete;

  reference(reference &&) = default;
  reference &operator=(reference &&) = default;

  using value_type = record<field<Symbols, typename std::iterator_traits<Iterators>::value_type>...>;
  operator value_type()
  {
    return map(*this, [](auto &&, auto &&v) { return v; });
  }
};

template <typename... Symbols, typename... Iterators>
class zip::iterator<zip::iterator_tag, record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, Iterators>...>
{
public:
  using iterator_record = record<field<Symbols, Iterators>...>;
  using iterator_record::operator=;

  // std::iterator_traits
  using iterator_category = zip::iterator_tag;
  using reference = zip::reference<iterator_record>;
  using value_type = typename reference::value_type;
  using pointer = reference *;
  using difference_type = std::ptrdiff_t;

  constexpr iterator(iterator_record it)
      : iterator_record{ it }
  {
  }
};

template <typename... Symbols, typename... Iterators>
class zip::iterator<std::input_iterator_tag, record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, Iterators>...>
{
public:
  using iterator_record = record<field<Symbols, Iterators>...>;
  using iterator_record::operator=;

  // std::iterator_traits
  using iterator_category = std::input_iterator_tag;
  using reference = zip::reference<iterator_record>;
  using value_type = typename reference::value_type;
  using pointer = reference *;
  using difference_type = std::ptrdiff_t;

  constexpr iterator(iterator_record it)
      : iterator_record{ it }
  {
  }
};

template <typename... Symbols, typename... Iterators>
class zip::iterator<std::output_iterator_tag, record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, Iterators>...>
{
public:
  using iterator_record = record<field<Symbols, Iterators>...>;
  using iterator_record::operator=;

  // std::iterator_traits
  using iterator_category = std::output_iterator_tag;
  using reference = zip::reference<iterator_record>;
  using value_type = typename reference::value_type;
  using pointer = reference *;
  using difference_type = std::ptrdiff_t;

  constexpr iterator(iterator_record it)
      : iterator_record{ it }
  {
  }
};

template <typename... Symbols, typename... Iterators>
class zip::iterator<std::forward_iterator_tag, record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, Iterators>...>
{
public:
  using iterator_record = record<field<Symbols, Iterators>...>;
  using iterator_record::operator=;

  // std::iterator_traits
  using iterator_category = std::forward_iterator_tag;
  using reference = zip::reference<iterator_record>;
  using value_type = typename reference::value_type;
  using pointer = reference *;
  using difference_type = std::ptrdiff_t;

  constexpr iterator(iterator_record it)
      : iterator_record{ it }
  {
  }
};

template <typename C, typename... S, typename... I>
constexpr bool operator==(zip::iterator<C, record<field<S, I>...>> l, zip::iterator<C, record<field<S, I>...>> r)
{
  // zip::iterators are considered equal if *any* of the zipped iterators compare equal
  // this is to catch cases where one sequence is shorter than the others
  return std::max({ get(S{}, l) == get(S{}, r)... });
}

template <typename... T>
constexpr bool operator!=(zip::iterator<T...> l, zip::iterator<T...> r)
{
  return !(l == r);
}

template <typename... T>
zip::iterator<T...> &operator++(zip::iterator<T...> &i)
{
  map(i, [](auto &&, auto &it) { return ++it; });
  return i;
}

template <typename... T>
zip::iterator<T...> operator++(zip::iterator<T...> &i, int)
{
  auto copy = i;
  ++i;
  return copy;
}

template <typename C, typename I>
zip::reference<I> operator*(zip::iterator<C, I> i)
{
  return zip::reference<I>{ i };
}

template <typename... Symbols, typename... Iterators>
class zip::iterator<std::bidirectional_iterator_tag, record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, Iterators>...>
{
public:
  using iterator_record = record<field<Symbols, Iterators>...>;
  using iterator_record::operator=;

  // std::iterator_traits
  using iterator_category = std::bidirectional_iterator_tag;
  using reference = zip::reference<iterator_record>;
  using value_type = typename reference::value_type;
  using pointer = reference *;
  using difference_type = std::ptrdiff_t;

  constexpr iterator(iterator_record it)
      : iterator_record{ it }
  {
  }

  iterator &operator--()
  {
    map(*this, [](auto &&, auto &it) { return --it; });
    return *this;
  }

  iterator operator--(int)
  {
    auto copy = *this;
    --*this;
    return copy;
  }
};

template <typename... Symbols, typename... Iterators>
class zip::iterator<std::random_access_iterator_tag, record<field<Symbols, Iterators>...>>
    : public record<field<Symbols, Iterators>...>
{
public:
  using iterator_record = record<field<Symbols, Iterators>...>;
  using iterator_record::operator=;

  // std::iterator_traits
  using iterator_category = std::random_access_iterator_tag;
  using reference = zip::reference<iterator_record>;
  using value_type = typename reference::value_type;
  using pointer = reference *;
  using difference_type = std::ptrdiff_t;

  constexpr iterator(iterator_record it)
      : iterator_record{ it }
  {
  }

  reference operator[](difference_type n) const
  {
    return reference{ *this + n };
  }

  iterator &operator+=(difference_type n)
  {
    map(*this, [n](auto &&, auto &it) { return it += n; });
    return *this;
  }

  iterator operator+(difference_type n) const
  {
    auto copy = *this;
    copy += n;
    return copy;
  }

  iterator &operator-=(difference_type n)
  {
    return *this += (-n);
  }

  iterator operator-(difference_type n) const
  {
    return *this + (-n);
  }

  iterator &operator--()
  {
    return *this -= 1;
  }

  iterator operator--(int)
  {
    auto copy = *this;
    *this -= 1;
    return copy;
  }

  // zipped iterators *should* only be ragged if two sequences have different size
  difference_type operator-(iterator other) const
  {
    return std::min({ get(Symbols{}, *this) - get(Symbols{}, other)... });
  }

  bool operator<(iterator const &other) const
  {
    return *this - other < 0;
  }

  bool operator<=(iterator const &other) const
  {
    return *this - other <= 0;
  }

  bool operator>(iterator const &other) const
  {
    return *this - other > 0;
  }

  bool operator>=(iterator const &other) const
  {
    return *this - other >= 0;
  }
};

template <typename I>
zip::iterator<std::random_access_iterator_tag, I> operator+(typename zip::iterator<std::random_access_iterator_tag, I>::difference_type n, zip::iterator<std::random_access_iterator_tag, I> &i)
{
  return i + n;
}

template <typename... Symbols, typename... ContainerRefs>
constexpr auto zip::operator()(field<Symbols, ContainerRefs>... container_ref_fields) const
{
  // We need this so we can zip temporaries too
  // test with bare arrays too
  //using container_record = record<field<Symbols,
  //  std::conditional_t<std::is_rvalue_reference<ContainerRefs>::value,
  //    std::remove_reference_t<ContainerRefs>,
  //    ContainerRefs>
  //  >...
  //>;
  //
  static_assert(std::min({ std::is_lvalue_reference<ContainerRefs>::value... }),
                "zipping temporaries not currently supported");
  using container_record = record<field<Symbols, ContainerRefs>...>;
  return zip::range<container_record>{ std::move(container_ref_fields)... };
}

constexpr struct zip zip = {};

} // namespace
} // namespace rekt

namespace std
{

template <typename I>
void swap(rekt::zip::reference<I> l, rekt::zip::reference<I> r)
{
  using value_type = typename rekt::zip::reference<I>::value_type;
  value_type tmp{ std::move(l) };
  l = std::move(r);
  r = std::move(tmp);
}

template <typename C, typename I>
void iter_swap(rekt::zip::iterator<C, I> l, rekt::zip::iterator<C, I> r)
{
  swap(*l, *r);
}

} // namespace std
