#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <rekt.hpp>

#include <string>

REKT_SYMBOLS(height, width, label);

// test with move only!!!

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
      rekt::field<struct label, std::string>
    >, rectangle_type>(), "make_record decays field types");
    static_assert(std::is_same<decltype(height(rectangle)), float&>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float&>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string&>(), "float field");
    height(rectangle) = 5.4F; // field is lvalue
    width(rectangle) = 2.6F; // field is lvalue
    label(rectangle).assign("Rect"); // field is lvalue

    auto const &crec = rectangle;
    using crectangle_type = decltype(crec);
    static_assert(rekt::has<struct height, crectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, crectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, crectangle_type>, "rectangles have label");
    static_assert(std::is_same<decltype(height(crec)), float const&>(), "float field");
    static_assert(std::is_same<decltype(width(crec)), float const&>(), "float field");
    static_assert(std::is_same<decltype(label(crec)), std::string const&>(), "float field");

  }

  {
    // forward_as_record
    auto const three = 3.F;
    auto four = 4.F;
    auto rectangle = rekt::forward_as_record(height = three, width = four, label = "rect"s);
    using rectangle_type = decltype(rectangle);
    static_assert(rekt::has<struct height, rectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, rectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, rectangle_type>, "rectangles have label");
    static_assert(std::is_same<rekt::record<
      rekt::field<struct height, float const&>,
      rekt::field<struct width, float&>,
      rekt::field<struct label, std::string&&>
    >, rectangle_type>(), "forward_as_record leaves references alone");
    static_assert(std::is_same<decltype(height(rectangle)), float const&>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float&>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string&&>(), "float field");
    //REQUIRE(&(height(rectangle)) == &three); // field is const lvalue
    auto const &also_three = height(rectangle);
    width(rectangle) = 2.6F; // field is lvalue
    auto l = label(rectangle); // field is rvalue
    REQUIRE(l == "rect");

    auto const &crec = rectangle;
    using crectangle_type = decltype(crec);
    static_assert(rekt::has<struct height, crectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, crectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, crectangle_type>, "rectangles have label");
    static_assert(std::is_same<decltype(height(crec)), float const&>(), "float field");
    static_assert(std::is_same<decltype(width(crec)), float&>(), "float field");
    static_assert(std::is_same<decltype(label(crec)), std::string&&>(), "float field");
  }

  {
    // forward_as_record
    auto const three = 3.F;
    auto four = 4.F;
    auto rectangle = rekt::forward_as_record(three - as_(height), let_(width) = four, label = "rect"s);
    using rectangle_type = decltype(rectangle);
    static_assert(rekt::has<struct height, rectangle_type>, "rectangles have height");
    static_assert(rekt::has<struct width, rectangle_type>, "rectangles have width");
    static_assert(rekt::has<struct label, rectangle_type>, "rectangles have label");
    static_assert(std::is_same<rekt::record<
      rekt::field<struct height, float const&>,
      rekt::field<struct width, float&>,
      rekt::field<struct label, std::string&&>
    >, rectangle_type>(), "forward_as_record leaves references alone");
    static_assert(std::is_same<decltype(height(rectangle)), float const&>(), "float field");
    static_assert(std::is_same<decltype(width(rectangle)), float&>(), "float field");
    static_assert(std::is_same<decltype(label(rectangle)), std::string&&>(), "float field");
    //REQUIRE(&(height(rectangle)) == &three); // field is const lvalue
    auto const &also_three = height(rectangle);
    width(rectangle) = 2.6F; // field is lvalue
    auto l = label(rectangle); // field is rvalue
    REQUIRE(l == "rect");
  }

}
