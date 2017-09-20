#include <catch.hpp>
#include <rekt.hpp>

#include <string>

REKT_SYMBOLS(height, width, label, dont);

#define STATIC_REQUIRE(...) static_assert(decltype(__VA_ARGS__)::value, #__VA_ARGS__)

// TODO test with move-only objects!!!
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

TEST_CASE("similar records with different factories")
{
  using namespace std::string_literals;

  auto const three = 3.F;
  auto four = 4.F;

  SECTION("make_record")
  {
    auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);
    STATIC_REQUIRE(rekt::has_(height, rectangle));
    STATIC_REQUIRE(rekt::has_(width, rectangle));
    STATIC_REQUIRE(rekt::has_(label, rectangle));
    //STATIC_REQUIRE(!rekt::has_(dont, rectangle));
    static_assert(!rekt::has<struct dont, decltype(rectangle)>, "");
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
    height(rectangle) = 5.4F; // field is lvalue
    width(rectangle) = 2.6F; // field is lvalue
    label(rectangle).assign("Rect"); // field is lvalue

    auto const &crec = rectangle;
    using crectangle_type = decltype(crec);
    static_assert(rekt::has<struct height, crectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, crectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, crectangle_type>, "rectangles have label");
    static_assert(!rekt::has<struct dont, crectangle_type>, "rectangles have no dont");
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
    using empty_type = decltype(empty);
    static_assert(!rekt::has<struct height, empty_type>, "empty defines no fields");
    static_assert(!rekt::has<struct width, empty_type>, "empty defines no fields");
    static_assert(!rekt::has<struct label, empty_type>, "empty defines no fields");
    static_assert(!rekt::has<struct dont, empty_type>, "empty defines no fields");

  }

  auto test_forward_as_record = [&](auto &&rectangle)
  {
    using rectangle_type = decltype(rectangle);
    static_assert(rekt::has<struct height, rectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, rectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, rectangle_type>, "rectangles have label");
    static_assert(!rekt::has<struct dont, rectangle_type>, "rectangles have no dont");
    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct height, float const &>,
                                   rekt::field<struct width, float &>,
                                   rekt::field<struct label, std::string &&>> &&,
                               rectangle_type>(),
                  "forward_as_record leaves references alone");
    static_assert(std::is_same<decltype(height(rectangle)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string &&>(), "string field");

    REQUIRE(&(height(rectangle)) == &three); // field is const lvalue
    width(rectangle) = 2.6F; // field is lvalue
    auto l = label(rectangle); // field is rvalue
    REQUIRE(l == "rect");

    auto const &crec = rectangle;
    using crectangle_type = decltype(crec);
    static_assert(rekt::has<struct height, crectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, crectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, crectangle_type>, "rectangles have label");
    static_assert(!rekt::has<struct dont, crectangle_type>, "rectangles have no dont");
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

TEST_CASE("assignment")
{
  using namespace std::string_literals;

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
    rectangle = rekt::make_record(height = 2, width = 5, label = "other rect"s);

    REQUIRE(height(rectangle) == 2);
    REQUIRE(width(rectangle) == 5);
    REQUIRE(label(rectangle) == "other rect");
  }
}

REKT_SYMBOLS(dimensions);

TEST_CASE("array sugar")
{
  auto cuboid = rekt::make_record(dimensions = { 3, 4, 5 }, label = "cuboid");
  REQUIRE(dimensions(cuboid) == (std::array<int, 3>{ 3, 4, 5 }));
  REQUIRE(label(cuboid) == (std::array<char, 7>{ 'c', 'u', 'b', 'o', 'i', 'd', '\0' }));
}

TEST_CASE("record composition functions")
{
  using namespace std::string_literals;

  // TODO more permutations, multiple override of one field

  auto const three = 3.F;
  auto four = 4.F;
  auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);

  {
    auto exclude_height = take(rectangle, width, label);
    using exclude_height_type = decltype(exclude_height);
    static_assert(!rekt::has<struct height, exclude_height_type>, "rectangles have height, but that field was not taken");
    static_assert(rekt::has<struct width, exclude_height_type>, "rectangles have width");
    static_assert(rekt::has<struct label, exclude_height_type>, "rectangles have label");
    static_assert(!rekt::has<struct dont, exclude_height_type>, "rectangles have no dont");

    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct width, float>,
                                   rekt::field<struct label, std::string>>,
                               exclude_height_type>(),
                  "take uses make_record");
  }

  {
    using override_add = decltype(merge(height = "four"s, rectangle, dimensions = 2));

    static_assert(rekt::has<struct height, override_add>, "rectangles have height, but that field is not usually a string");
    static_assert(rekt::has<struct dimensions, override_add>, "augmented with dimensions");
    static_assert(!rekt::has<struct dont, override_add>, "rectangles have no dont");
  }

  {
    using override_height = decltype(rectangle ^ (height = "four"s));

    static_assert(rekt::has<struct height, override_height>, "rectangles have height, but that field is not usually a string");
    static_assert(std::is_same<std::string &&, rekt::field_type_for<struct height, override_height>>(),
                  "the height field is overridden to a string");
    static_assert(rekt::has<struct width, override_height>, "rectangles have width");
    static_assert(rekt::has<struct label, override_height>, "rectangles have label");
    static_assert(!rekt::has<struct dont, override_height>, "rectangles have no dont");

    REQUIRE(height(rectangle ^ (height = "four"s)) == "four"s);
    REQUIRE(width(rectangle ^ (height = "four"s)) == width(rectangle));
    REQUIRE(label(rectangle ^ (height = "four"s)) == label(rectangle));
  }

  {
    using add_dimensions = decltype(rectangle & (dimensions = std::make_pair(height, width)));

    static_assert(rekt::has<struct dimensions, add_dimensions>, "augmented with field for dimensions");
    static_assert(!rekt::has<struct dont, add_dimensions>, "rectangles have no dont");

    static_assert(std::is_same<std::pair<struct height, struct width> &&,
                               rekt::field_type_for<struct dimensions, add_dimensions>>(),
                  "the dimensions field is defined as a pair of symbols naming the axes which this rectangle spans");
    REQUIRE(dimensions(rectangle & (dimensions = std::make_pair(height, width))) == std::make_pair(height, width));
  }
}

REKT_SYMBOLS(name, age, friends);

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

  static auto get_friends(person_t const &p)
  {
    return p.name.size();
  }
};

auto get(rekt::properties, person_t const &)
{
  return make_record(name = &person_t::name,
                     age = rekt::get_set(&person_t::get_age, &person_t::set_age),
                     friends = person_t::get_friends);
}

TEST_CASE("introspection functions")
{
  static_assert(rekt::has<struct name, person_t>(), "person_t defines name");
  static_assert(rekt::has<struct age, person_t>(), "person_t defines age");
  static_assert(rekt::has<struct friends, person_t>(), "person_t defines age");
  static_assert(!rekt::has<struct dont, person_t>(), "person_t doesn't define dont");

  person_t genos = { "genos", 19 };

  REQUIRE(name(genos) == "genos");
  REQUIRE(age(genos) == 19);
  age(genos) = 20;
  REQUIRE(age(genos) == 20);
}
