#include "move_only.hpp"
#include <catch.hpp>
#include <rekt.hpp>

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

REKT_SYMBOLS(height, width, label, dont);

// helper for asserting non-constexpr integer_constants
#define STATIC_REQUIRE(...) static_assert(decltype(__VA_ARGS__)::value, #__VA_ARGS__)

// TODO break these into smaller cases with tags

// TODO find a home for these:
//static_assert(rekt::has<struct height, rekt::field<struct height, int&&>>, "a field has a field defined for its symbol");
//static_assert(rekt::has<struct height, rekt::field<struct height, int&&> const>, "a field has a field defined for its symbol");
//static_assert(rekt::has<struct height, rekt::field<struct height, int&&> &>, "a field has a field defined for its symbol");
//static_assert(rekt::has<struct height, rekt::field<struct height, int&&> const&>, "a field has a field defined for its symbol");
//static_assert(rekt::has<struct height, rekt::field<struct height, int&&> &&>, "a field has a field defined for its symbol");
//
//static_assert(!rekt::has<struct width, rekt::field<struct height, int&&>>, "a field has a field defined for its symbol");
//static_assert(!rekt::has<struct width, rekt::field<struct height, int&&> const>, "a field has a field defined for its symbol");
//static_assert(!rekt::has<struct width, rekt::field<struct height, int&&> &>, "a field has a field defined for its symbol");
//static_assert(!rekt::has<struct width, rekt::field<struct height, int&&> const&>, "a field has a field defined for its symbol");
//static_assert(!rekt::has<struct width, rekt::field<struct height, int&&> &&>, "a field has a field defined for its symbol");

using namespace std::string_literals;

TEST_CASE("similar records with different factories")
{
  auto const three = 3.F;
  auto four = 4.F;

  SECTION("make_record")
  {
    auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);
    STATIC_REQUIRE(height.in(rectangle));
    STATIC_REQUIRE(width.in(rectangle));
    STATIC_REQUIRE(label.in(rectangle));
    STATIC_REQUIRE(dont.not_in(rectangle));
    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct height, float>,
                                   rekt::field<struct width, float>,
                                   rekt::field<struct label, std::string>>,
                               decltype(rectangle)>(),
                  "make_record decays field types");
    static_assert(std::is_same<decltype(height(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string &>(), "float field");
    REQUIRE(height(rectangle) == 3.F);
    REQUIRE(width(rectangle) == 4.F);
    REQUIRE(label(rectangle) == "rect");
    height(rectangle) = 5.4F;        // field is lvalue
    width(rectangle) = 2.6F;         // field is lvalue
    label(rectangle).assign("Rect"); // field is lvalue

    auto const &crec = rectangle;
    STATIC_REQUIRE(height.in(crec));
    STATIC_REQUIRE(width.in(crec));
    STATIC_REQUIRE(label.in(crec));
    STATIC_REQUIRE(dont.not_in(crec));
    static_assert(std::is_same<decltype(height(crec)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(crec)), float const &>(), "float field");
    static_assert(std::is_same<decltype(label(crec)), std::string const &>(), "string field");
    REQUIRE(height(crec) == 5.4F);
    REQUIRE(width(crec) == 2.6F);
    REQUIRE(label(crec) == "Rect");
  }

  SECTION("empty make_record")
  {
    auto empty = rekt::make_record();
    STATIC_REQUIRE(height.not_in(empty));
    STATIC_REQUIRE(width.not_in(empty));
    STATIC_REQUIRE(label.not_in(empty));
    STATIC_REQUIRE(dont.not_in(empty));
  }

  auto test_forward_as_record = [&](auto &&rectangle) {
    STATIC_REQUIRE(height.in(rectangle));
    STATIC_REQUIRE(width.in(rectangle));
    STATIC_REQUIRE(label.in(rectangle));
    STATIC_REQUIRE(dont.not_in(rectangle));
    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct height, float const &>,
                                   rekt::field<struct width, float &>,
                                   rekt::field<struct label, std::string &&>> &&,
                               decltype(rectangle)>(),
                  "forward_as_record leaves references alone");
    static_assert(std::is_same<decltype(height(rectangle)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string &&>(), "string field");

    REQUIRE(&(height(rectangle)) == &three); // field is const lvalue
    width(rectangle) = 2.6F;                 // field is lvalue
    auto l = label(rectangle);               // field is rvalue
    REQUIRE(l == "rect");

    auto const &crec = rectangle;
    STATIC_REQUIRE(height.in(crec));
    STATIC_REQUIRE(width.in(crec));
    STATIC_REQUIRE(label.in(crec));
    STATIC_REQUIRE(dont.not_in(crec));
    static_assert(std::is_same<decltype(height(crec)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(crec)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(crec)), std::string &&>(), "float field");
  };

  SECTION("forward_as_record")
  {
    test_forward_as_record(rekt::forward_as_record(height = three, width = four, label = "rect"s));
    test_forward_as_record(rekt::forward_as_record(three - as_(height), let_(width) = four, label = "rect"s));
  }
}

TEST_CASE("move_only")
{
  auto m0 = make_record(label = move_only{ "m" });
  static_assert(!std::is_copy_constructible<decltype(m0)>{}, "");
  auto m1 = std::move(m0);
  REQUIRE(label(m0).name() == "<moved>");
  REQUIRE(label(m1).name() == "m");

  // verify that a lambda can be a field
  auto f = make_record(label = [] { return "f"; });
}

TEST_CASE("assignment")
{
  auto const three = 3.F;
  auto four = 4.F;

  /*
  // not yet implemented
  SECTION("tie style destructuring")
  {
    float also_three;
    int also_four;
    std::string also_rect;

    auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);
    rekt::tie(height = also_three, width = also_four, label = also_rect) = rectangle;

    REQUIRE(also_three == 3.F);
    REQUIRE(also_four == 4);
    REQUIRE(also_rect == "rect");
  }
  */
  SECTION("copying")
  {
    auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);
    rectangle = rectangle; // just to make sure this compiles
    rectangle = rekt::make_record(height = 2.F, width = 5.F, label = "other rect"s);

    REQUIRE(height(rectangle) == 2);
    REQUIRE(width(rectangle) == 5);
    REQUIRE(label(rectangle) == "other rect");
  }
}

REKT_SYMBOLS(dimensions);

TEST_CASE("array sugar")
{
  rekt::record<rekt::field<struct label, int[]>> naked;
  label(naked) = nullptr;
  auto cuboid = rekt::make_record(dimensions = { 3, 4, 5 }, label = "cuboid");
  REQUIRE(dimensions(cuboid) == (std::array<int, 3>{ 3, 4, 5 }));
  REQUIRE(label(cuboid) == (std::array<char, 7>{ 'c', 'u', 'b', 'o', 'i', 'd', '\0' }));
}

TEST_CASE("record composition functions")
{
  // TODO more permutations, multiple override of one field

  auto const three = 3.F;
  auto four = 4.F;
  auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);

  SECTION("remove rectangle's 'height'")
  {
    auto exclude_height = take(rectangle, width, label);
    STATIC_REQUIRE(height.not_in(exclude_height));
    STATIC_REQUIRE(width.in(exclude_height));
    STATIC_REQUIRE(label.in(exclude_height));
    STATIC_REQUIRE(dont.not_in(exclude_height));
    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct width, float>,
                                   rekt::field<struct label, std::string>>,
                               decltype(exclude_height)>(),
                  "take uses make_record");
  }

  {
    using override_add = decltype(merge(height = "four"s, rectangle, dimensions = 2));

    static_assert(rekt::has<struct height, override_add>, "rectangles have height, but that field is not usually a string");
    static_assert(rekt::has<struct dimensions, override_add>, "augmented with dimensions");
    static_assert(!rekt::has<struct dont, override_add>, "rectangles have no dont");
  }

  SECTION("override rectangle's 'height' with a string")
  {
    STATIC_REQUIRE(height.in(rectangle ^ (height = "four"s)));
    STATIC_REQUIRE(width.in(rectangle ^ (height = "four"s)));
    STATIC_REQUIRE(label.in(rectangle ^ (height = "four"s)));
    STATIC_REQUIRE(dont.not_in(rectangle ^ (height = "four"s)));

    static_assert(std::is_same<decltype(height(rectangle ^ (height = "four"s))),
                               std::string &&>(),
                  "the height field is overridden to a string");

    REQUIRE(height(rectangle ^ (height = "four"s)) == "four"s);
    REQUIRE(width(rectangle ^ (height = "four"s)) == width(rectangle));
    REQUIRE(label(rectangle ^ (height = "four"s)) == label(rectangle));
  }

  SECTION("augment rectangle with 'dimensions'")
  {
    STATIC_REQUIRE(dimensions.in(rectangle & (dimensions = std::make_pair(height, width))));
    STATIC_REQUIRE(dont.not_in(rectangle & (dimensions = std::make_pair(height, width))));

    using add_dimensions = decltype(rectangle & (dimensions = std::make_pair(height, width)));
    static_assert(std::is_same<std::pair<struct height, struct width> &&,
                               rekt::field_type_for<struct dimensions, add_dimensions>>(),
                  "the dimensions field is defined as a pair of symbols naming the axes which this rectangle spans");
    REQUIRE(dimensions(rectangle & (dimensions = std::make_pair(height, width))) == std::make_pair(height, width));
  }
}

REKT_SYMBOLS(name, age, friends);

namespace hero
{
REKT_SYMBOLS(name);

std::unordered_map<std::string, std::string> name_registry;
}

struct person_t
{
  std::string name;
  int age_;

  int get_age()
  {
    return age_;
  }
  void set_age(int age)
  {
    age_ = age;
  }

  static auto &get_freinds(person_t const &p)
  {
    static std::unordered_map<std::string, std::vector<std::string>> friends_server_;
    return friends_server_[p.name];
  }
};

auto get(rekt::properties, person_t const &)
{
  return make_record(name = &person_t::name,
                     hero::name = [](person_t const &p) {
                       // FIXME I want to be able to do this:
                       // auto it = hero::name_registry.find(name(p));
                       auto it = hero::name_registry.find(p.name);
                       return it != hero::name_registry.end()
                           ? it->second
                           : "<no hero name>"s;
                     },
                     age = rekt::get_set(&person_t::get_age, &person_t::set_age), friends = person_t::get_freinds);
}

TEST_CASE("introspection functions")
{
  person_t genos = { "genos", 19 };

  STATIC_REQUIRE(name.in(genos));
  STATIC_REQUIRE(hero::name.in(genos));
  STATIC_REQUIRE(age.in(genos));
  STATIC_REQUIRE(friends.in(genos));
  STATIC_REQUIRE(dont.not_in(genos));

  REQUIRE(name(genos) == "genos");
  REQUIRE(age(genos) == 19);
  age(genos) = 20;
  REQUIRE(age(genos) == 20);

  REQUIRE(rekt::nameof(age) == "age");
  REQUIRE(rekt::nameof(hero::name) == "hero::name");
}

TEST_CASE("unpacking")
{
  auto genos_json = nlohmann::json{
    { "name", "genos" },
    { "age", 19 }
  };
  auto genos = rekt::make_record(name = "genos"s, age = 19);

  nlohmann::json packed;
  rekt::pack(genos, &packed);
  REQUIRE(packed == genos_json);

  decltype(genos) unpacked;
  rekt::unpack(genos_json, &unpacked);
  REQUIRE(name(genos) == name(unpacked));
  REQUIRE(age(genos) == age(unpacked));
}

TEST_CASE("iteration")
{
  std::vector<int> indices{ 1, 2, 0 };
  std::string strings[] = { "world", "!", "hello" };

  REKT_SYMBOLS(index, string, expected);
  auto zipped = rekt::zip(index = indices, string = strings);
  std::sort(zipped.begin(), zipped.end(), [&](auto const &l, auto const &r) {
    return index(l) < index(r);
  });

  REQUIRE(std::is_sorted(indices.begin(), indices.end()));

  std::vector<std::string> sorted_by_index{ "hello", "world", "!" };
  for (auto r : rekt::zip(string = strings, expected = sorted_by_index))
  {
    REQUIRE(string(r) == expected(r));
  }
}
