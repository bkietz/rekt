#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <rekt.hpp>

#include <string>

REKT_SYMBOLS(height, width, label);

// DODO test with move-only objects!!!
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

  {
    // make_record
    auto const three = 3.F;
    auto four = 4.F;
    auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);
    using rectangle_type = decltype(rectangle);
    static_assert(rekt::has<struct height, rectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, rectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, rectangle_type>, "rectangles have label");
    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct height, float>,
                                   rekt::field<struct width, float>,
                                   rekt::field<struct label, std::string>>,
                               rectangle_type>(),
                  "make_record decays field types");
    static_assert(std::is_same<decltype(height(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string &>(), "float field");
    height(rectangle) = 5.4F; // field is lvalue
    width(rectangle) = 2.6F; // field is lvalue
    label(rectangle).assign("Rect"); // field is lvalue

    auto const &crec = rectangle;
    using crectangle_type = decltype(crec);
    static_assert(rekt::has<struct height, crectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, crectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, crectangle_type>, "rectangles have label");
    static_assert(std::is_same<decltype(height(crec)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(crec)), float const &>(), "float field");
    static_assert(std::is_same<decltype(label(crec)), std::string const &>(), "float field");
  }

  auto const three = 3.F;
  auto four = 4.F;
  auto test_forward_as_record = [&](auto &&rectangle) {
    // forward_as_record
    using rectangle_type = decltype(rectangle);
    static_assert(rekt::has<struct height, rectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, rectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, rectangle_type>, "rectangles have label");
    static_assert(std::is_same<rekt::record<
                                   rekt::field<struct height, float const &>,
                                   rekt::field<struct width, float &>,
                                   rekt::field<struct label, std::string &&>> &&,
                               rectangle_type>(),
                  "forward_as_record leaves references alone");
    static_assert(std::is_same<decltype(height(rectangle)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string &&>(), "float field");

    REQUIRE(&(height(rectangle)) == &three); // field is const lvalue
    width(rectangle) = 2.6F; // field is lvalue
    auto l = label(rectangle); // field is rvalue
    REQUIRE(l == "rect");

    auto const &crec = rectangle;
    using crectangle_type = decltype(crec);
    static_assert(rekt::has<struct height, crectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, crectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, crectangle_type>, "rectangles have label");
    static_assert(std::is_same<decltype(height(crec)), float const &>(), "float field");
    static_assert(std::is_same<decltype(width(crec)), float &>(), "float field");
    static_assert(std::is_same<decltype(label(crec)), std::string &&>(), "float field");
  };

  test_forward_as_record(rekt::forward_as_record(height = three, width = four, label = "rect"s));
  test_forward_as_record(rekt::forward_as_record(three - as_(height), let_(width) = four, label = "rect"s));
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

  auto const three = 3.F;
  auto four = 4.F;
  auto rectangle = rekt::make_record(height = three, width = four, label = "rect"s);

  {
    auto exclude_height = take(rectangle, width, label);
    using exclude_height_type = decltype(exclude_height);
    static_assert(!rekt::has<struct height, exclude_height_type>, "rectangles have height, but that field was not taken");
    static_assert(rekt::has<struct width, exclude_height_type>, "rectangles have width");
    static_assert(rekt::has<struct label, exclude_height_type>, "rectangles have label");

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
  }

  {
    using override_height = decltype(rectangle ^ (height = "four"s));

    static_assert(rekt::has<struct height, override_height>, "rectangles have height, but that field is not usually a string");
    static_assert(std::is_same<std::string &&, rekt::field_type_for<struct height, override_height>>(),
                  "the height field is overridden to a string");
    static_assert(rekt::has<struct width, override_height>, "rectangles have width");
    static_assert(rekt::has<struct label, override_height>, "rectangles have label");

    REQUIRE(height(rectangle ^ (height = "four"s)) == "four"s);
    REQUIRE(width(rectangle ^ (height = "four"s)) == width(rectangle));
    REQUIRE(label(rectangle ^ (height = "four"s)) == label(rectangle));
  }

  {
    using add_dimensions = decltype(rectangle & (dimensions = std::make_pair(height, width)));

    static_assert(rekt::has<struct dimensions, add_dimensions>, "augmented with field for dimensions");

    auto t = rekt::type_c<decltype(dimensions(rekt::type_c<add_dimensions>.declval()))>;
    auto t2 = rekt::type_c<decltype(std::make_pair(height, width))>;
    static_assert(std::is_same<std::pair<struct height, struct width> &&,
                               rekt::field_type_for<struct dimensions, add_dimensions>>(),
                  "the dimensions field is defined as a pair of symbols naming the axes which this rectangle spans");
    REQUIRE(dimensions(rectangle & (dimensions = std::make_pair(height, width))) == std::make_pair(height, width));
  }
}

REKT_SYMBOLS(name);

struct person_t
{
  std::string name;
};

auto get(rekt::properties, person_t const &)
{
  return make_record(name = &person_t::name);
}

TEST_CASE("introspection functions")
{
  static_assert(rekt::has_property_for<struct name, person_t>(), "person_t defines name");

  person_t genos = { "genos" };

  REQUIRE(name(genos) == "genos");
}