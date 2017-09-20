#pragma once

#include <rekt/detail/map_macro.hpp>

#define REKT_SYMBOL(SYMBOL)                       \
  struct SYMBOL : rekt::symbol<struct SYMBOL>     \
  {                                               \
    using rekt::symbol<struct SYMBOL>::operator=; \
  };                                              \
  constexpr struct SYMBOL SYMBOL = {}

#define REKT_SYMBOLS(...) REKT_MAP(REKT_SYMBOL, (;), __VA_ARGS__)