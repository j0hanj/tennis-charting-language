#include "viz/court.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>

#include "parser/parser.hpp"

using tcl::parser::parse;
using tcl::viz::render_svg;

namespace {

std::string svg_for(std::string_view s) {
  auto r = parse(s);
  REQUIRE(r.point.has_value());
  return render_svg(*r.point, s);
}

}  // namespace

TEST_CASE("render_svg produces a well-formed svg", "[viz]") {
  const std::string svg = svg_for("4ffbbf*");
  CHECK(svg.rfind("<svg", 0) == 0);
  CHECK(svg.find("</svg>") != std::string::npos);
  CHECK(svg.find("<polyline") != std::string::npos);
  CHECK(svg.find("4ffbbf*") != std::string::npos); // the source is in the title
}

TEST_CASE("a winner ends gold, an error ends red", "[viz]") {
  CHECK(svg_for("4ffbbf*").find("#f4c542") != std::string::npos);
  const std::string err = svg_for("4bf@n");
  CHECK(err.find("#e5484d") != std::string::npos);
  CHECK(err.find("net") != std::string::npos); // tag mentions where
}

TEST_CASE("more shots means a longer path", "[viz]") {
  auto count_points = [](const std::string& svg) {
    const auto a = svg.find("points='");
    const auto b = svg.find('\'', a + 8);
    return std::count(svg.begin() + static_cast<long>(a),
                      svg.begin() + static_cast<long>(b), ' ');
  };
  CHECK(count_points(svg_for("4f*")) < count_points(svg_for("4ffbbbf*")));
}
