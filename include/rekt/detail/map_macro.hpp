/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define REKT_EVAL(...) REKT_EVAL_1024(__VA_ARGS__)
#define REKT_EVAL_1024(...) REKT_EVAL_512(REKT_EVAL_512(__VA_ARGS__))
#define REKT_EVAL_512(...) REKT_EVAL_256(REKT_EVAL_256(__VA_ARGS__))
#define REKT_EVAL_256(...) REKT_EVAL_128(REKT_EVAL_128(__VA_ARGS__))
#define REKT_EVAL_128(...) REKT_EVAL_64(REKT_EVAL_64(__VA_ARGS__))
#define REKT_EVAL_64(...) REKT_EVAL_32(REKT_EVAL_32(__VA_ARGS__))
#define REKT_EVAL_32(...) REKT_EVAL_16(REKT_EVAL_16(__VA_ARGS__))
#define REKT_EVAL_16(...) REKT_EVAL_8(REKT_EVAL_8(__VA_ARGS__))
#define REKT_EVAL_8(...) REKT_EVAL_4(REKT_EVAL_4(__VA_ARGS__))
#define REKT_EVAL_4(...) REKT_EVAL_2(REKT_EVAL_2(__VA_ARGS__))
#define REKT_EVAL_2(...) REKT_EVAL_1(REKT_EVAL_1(__VA_ARGS__))
#define REKT_EVAL_1(...) __VA_ARGS__

#define REKT_CONCAT(a, b) REKT_CONCAT_IMPL(a, b)
#define REKT_CONCAT_IMPL(a, b) a##b
#define REKT_IGNORE(...)
#define REKT_PASS(...) __VA_ARGS__

#define REKT_DEFER_1(m) m REKT_IGNORE()
#define REKT_DEFER_2(m) m REKT_IGNORE REKT_IGNORE()()

#define REKT_ZERO(...) 0
#define REKT_FIRST(a, ...) a
#define REKT_SECOND(a, b, ...) b

#ifdef _MSC_VER
#define REKT_SECOND_OR_ZERO(...) REKT_EVAL_1(REKT_DEFER_1(REKT_SECOND)(__VA_ARGS__, REKT_ZERO()))
#else // _MSC_VER
#define REKT_SECOND_OR_ZERO(...) REKT_SECOND(__VA_ARGS__, REKT_ZERO())
#endif // _MSC_VER

#define REKT_NOT(x) REKT_SECOND_OR_ZERO(REKT_CONCAT(REKT_NOT_IMPL_, x))
#define REKT_NOT_IMPL_0 , 1
#define REKT_TRUTHY(x) REKT_NOT(REKT_NOT(x))
#define REKT_EMPTY(...) REKT_NOT(REKT_FIRST(REKT_ZERO __VA_ARGS__)())

#ifdef _MSC_VER
#define REKT_IF_ELSE(condition) \
  REKT_DEFER_1(REKT_CONCAT)     \
  (REKT_IF_ELSE_IMPL_, REKT_TRUTHY(condition))
#else // _MSC_VER
#define REKT_IF_ELSE(condition) REKT_CONCAT(REKT_IF_ELSE_IMPL_, REKT_TRUTHY(condition))
#endif // _MSC_VER

#define REKT_IF_ELSE_IMPL_1(...) __VA_ARGS__ REKT_IGNORE
#define REKT_IF_ELSE_IMPL_0(...) REKT_PASS

#define REKT_MAP_IMPL(m, sep, first, ...)      \
  m(first)                                     \
      REKT_IF_ELSE(REKT_EMPTY(__VA_ARGS__))()( \
          REKT_PASS sep                        \
              REKT_DEFER_2(REKT_MAP_IMPL_DEFERRED)()(m, sep, __VA_ARGS__))

#define REKT_MAP_IMPL_DEFERRED() REKT_MAP_IMPL

#define REKT_MAP(m, sep, ...) REKT_EVAL(REKT_MAP_IMPL(m, sep, __VA_ARGS__))

#if (!defined(NDEBUG) || defined(_DEBUG))

// macro asserts...

#if REKT_SECOND_OR_ZERO() != 0
#error "SECOND_OR_ZERO() != 0"
#endif

#if REKT_SECOND_OR_ZERO(1) != 0
#error "SECOND_OR_ZERO(1) != 0"
#endif

#if REKT_SECOND_OR_ZERO(1, 2, 3) != 2
#error "SECOND_OR_ZERO(1, 2, 3) != 2"
#endif

#if REKT_NOT(1) != false
#error "NOT(1) != false"
#endif

#if REKT_NOT(0) != true
#error "NOT(0) != true"
#endif

#if REKT_NOT(3) != false
#error "NOT(3) != false"
#endif

#if REKT_TRUTHY(1) != true
#error "TRUTHY(1) != true"
#endif

#if REKT_TRUTHY(0) != false
#error "TRUTHY(0) != false"
#endif

#if REKT_TRUTHY(3) != true
#error "TRUTHY(3) != true"
#endif

#if REKT_EMPTY() != true
#error "EMPTY() != true"
#endif

#if REKT_EMPTY(1) != false
#error "EMPTY(1) != false"
#endif

#if REKT_EMPTY(0, 1, 2) != false
#error "EMPTY(0, 1, 2) != false"
#endif

#if REKT_IF_ELSE(3)(true)(false) != true
#error "IF_ELSE(3)(true)(false) != true"
#endif

#if REKT_IF_ELSE(glongfockulus)(true)(false) != true
#error "IF_ELSE(glongfockulus)(true)(false) != true"
#endif

#if REKT_IF_ELSE(0)(true)(false) != false
#error "IF_ELSE(0)(true)(false) != false"
#endif

#if REKT_IF_ELSE(REKT_EMPTY())(true)(false) != true
#error "IF_ELSE(EMPTY())(true)(false) != true"
#endif

#if REKT_IF_ELSE(REKT_EMPTY(1, 2))(true)(false) != false
#error "IF_ELSE(EMPTY(1, 2))(true)(false) != false"
#endif

#endif // !defined(NDEBUG) || defined(_DEBUG)
