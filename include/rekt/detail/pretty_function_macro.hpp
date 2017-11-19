/// Copyright (c) Benjamin Kietzman (github.com/bkietz)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifdef _MSC_VER

#define REKT_PRETTY_FUNCTION __FUNCSIG__

namespace rekt
{
namespace
{
namespace pretty_function
{

// in Visual C++ the names of class types are decorated with 'struct ' or 'class '
constexpr bool class_names_bare = false;

} // namespace pretty_function
} // namespace
} // namespace rekt

#else

#define REKT_PRETTY_FUNCTION __PRETTY_FUNCTION__

namespace rekt
{
namespace
{
namespace pretty_function
{

constexpr bool class_names_bare = true;

} // namespace pretty_function
} // namespace
} // namespace rekt

#endif // _MSC_VER
