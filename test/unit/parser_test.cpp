#include "parser/parser.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using tcl::ast::Ender;
using tcl::ast::ErrorLoc;
using tcl::parser::parse;

TEST_CASE("a plain point parses into serve + rally + winner", "[parser]") {
  auto r = parse("4ffbbf*");
  REQUIRE(r.ok());
  REQUIRE(r.point.has_value());

  const auto& p = *r.point;
  CHECK(p.serve.direction == 4);
  REQUIRE(p.rally.size() == 5);
  CHECK(p.rally[0].type == 'f');
  CHECK(p.rally[2].type == 'b');
  REQUIRE(p.outcome.has_value());
  CHECK(p.outcome->ender == Ender::kWinner);
}

TEST_CASE("an ace is just a serve and a winner", "[parser]") {
  auto r = parse("4*");
  REQUIRE(r.ok());
  const auto& p = *r.point;
  CHECK(p.serve.direction == 4);
  CHECK(p.rally.empty());
  REQUIRE(p.outcome.has_value());
  CHECK(p.outcome->ender == Ender::kWinner);
}

TEST_CASE("direction, depth and position hang off the right shots", "[parser]") {
  auto r = parse("5r37b+3l2o=1r#");
  REQUIRE(r.ok());
  const auto& p = *r.point;
  CHECK(p.serve.direction == 5);
  REQUIRE(p.rally.size() == 5);

  CHECK(p.rally[0].type == 'r');
  CHECK(p.rally[0].direction == 3);
  CHECK(p.rally[0].depth == 7);

  CHECK(p.rally[1].type == 'b');
  CHECK(p.rally[1].position == '+');
  CHECK(p.rally[1].direction == 3);

  CHECK(p.rally[3].position == '=');
  CHECK(p.rally[3].direction == 1);

  REQUIRE(p.outcome.has_value());
  CHECK(p.outcome->ender == Ender::kForcedError);
}

TEST_CASE("error endings carry the location", "[parser]") {
  auto a = parse("4fn@");
  REQUIRE(a.ok());
  CHECK(a.point->outcome->ender == Ender::kUnforcedError);
  CHECK(a.point->outcome->where == ErrorLoc::kNet);

  auto b = parse("4bfw#");
  REQUIRE(b.ok());
  CHECK(b.point->outcome->ender == Ender::kForcedError);
  CHECK(b.point->outcome->where == ErrorLoc::kWide);
}

TEST_CASE("a missing serve direction is flagged but the rest still parses", "[parser]") {
  auto r = parse("ff*");
  CHECK_FALSE(r.ok());
  REQUIRE(r.diagnostics.size() == 1);
  CHECK(r.diagnostics[0].offset == 0);

  REQUIRE(r.point.has_value());
  CHECK_FALSE(r.point->serve.direction.has_value());
  CHECK(r.point->rally.size() == 2);
  CHECK(r.point->outcome->ender == Ender::kWinner);
}

TEST_CASE("a point with no ending marker is flagged", "[parser]") {
  auto r = parse("4ff");
  CHECK_FALSE(r.ok());
  REQUIRE(r.diagnostics.size() == 1);
  CHECK(r.point.has_value());
  CHECK_FALSE(r.point->outcome.has_value());
  CHECK(r.point->rally.size() == 2);
}

TEST_CASE("an unknown character is reported once, from the lexer", "[parser]") {
  auto r = parse("4fQf*");
  REQUIRE(r.diagnostics.size() == 1);
  CHECK(r.diagnostics[0].offset == 2);

  // the Q is dropped, the rest parses
  REQUIRE(r.point.has_value());
  CHECK(r.point->rally.size() == 2);
  CHECK(r.point->outcome->ender == Ender::kWinner);
}

TEST_CASE("trailing junk after the ending is flagged", "[parser]") {
  auto r = parse("4f*f");
  CHECK_FALSE(r.ok());
  REQUIRE(r.diagnostics.size() == 1);
  CHECK(r.diagnostics[0].offset == 3);
  CHECK(r.point->outcome->ender == Ender::kWinner);
}

TEST_CASE("to_string dumps the tree", "[parser]") {
  auto r = parse("4f3bd@");
  REQUIRE(r.point.has_value());
  const std::string dump = tcl::ast::to_string(*r.point);
  CHECK(dump.find("serve dir=4") != std::string::npos);
  CHECK(dump.find("shot f dir=3") != std::string::npos);
  CHECK(dump.find("end unforced deep") != std::string::npos);
}
