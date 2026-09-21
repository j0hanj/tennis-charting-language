#include "viz/court.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>

#include <vector>

#include "parser/parser.hpp"

using tcl::parser::parse;
using tcl::viz::render_match_svg;
using tcl::viz::render_svg;

namespace {

std::string svg_for(std::string_view s) {
  auto r = parse(s);
  REQUIRE(r.point.has_value());
  return render_svg(*r.point, s);
}

tcl::ast::Point point_for(std::string_view s) {
  auto r = parse(s);
  REQUIRE(r.point.has_value());
  return *r.point;
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
  const std::string err = svg_for("4bfn@");
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

TEST_CASE("render_match_svg draws every point on one court", "[viz]") {
  const std::vector<tcl::ast::Point> points{point_for("4ffbbf*"), point_for("4bfn@"),
                                            point_for("5r37b+3l2o=1r#")};
  const std::string svg = render_match_svg(points, "test match");

  CHECK(svg.rfind("<svg", 0) == 0);
  CHECK(svg.find("</svg>") != std::string::npos);
  CHECK(svg.find("<rect") != std::string::npos); // the court
  CHECK(svg.find("test match") != std::string::npos);
  CHECK(svg.find("3 points") != std::string::npos);
  CHECK(svg.find("1 winners") != std::string::npos); // the one "*" point
  CHECK(svg.find("2 errors") != std::string::npos);  // "@" and "#"
  // one gold-ish and one red-ish outcome dot should show up
  CHECK(svg.find("#f4c542") != std::string::npos);
  CHECK(svg.find("#e5484d") != std::string::npos);
}

TEST_CASE("render_match_svg clips a title that would run off the edge", "[viz]") {
  const std::vector<tcl::ast::Point> points{point_for("4*")};
  const std::string long_title(80, 'x');
  const std::string svg = render_match_svg(points, long_title);
  CHECK(svg.find(long_title) == std::string::npos); // the full thing shouldn't appear
  CHECK(svg.find("\xe2\x80\xa6") != std::string::npos); // the ellipsis should
}

TEST_CASE("render_match_svg on an empty match still makes a valid svg", "[viz]") {
  const std::string svg = render_match_svg({}, "nothing");
  CHECK(svg.rfind("<svg", 0) == 0);
  CHECK(svg.find("</svg>") != std::string::npos);
  CHECK(svg.find("0 points") != std::string::npos);
}

TEST_CASE("a missed serve is drawn as a fault, not a winner", "[viz]") {
  const std::string svg = svg_for("6d");
  CHECK(svg.find("fault (deep)") != std::string::npos);
  CHECK(svg.find("#e5484d") != std::string::npos);  // red end marker
  CHECK(svg.find("fill='#f4c542'") == std::string::npos);  // no gold winner dot (the path stroke is gold either way)
}
