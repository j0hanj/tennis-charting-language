#include "ir/shot_table.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using tcl::ast::Ender;
using tcl::ir::flatten;
using tcl::ir::to_csv_line;
using tcl::match::PointRow;

namespace {

PointRow row(int pt, int server, std::string first, std::string second, int winner) {
  PointRow r;
  r.match_id = "m";
  r.pt = pt;
  r.server = server;
  r.first = std::move(first);
  r.second = std::move(second);
  r.pt_winner = winner;
  return r;
}

}  // namespace

TEST_CASE("a rally alternates hitters starting with the server", "[ir]") {
  // serve + 5 shots = 6 shots, so the returner hits the last one and wins it
  const auto f = flatten({row(1, 1, "4ffbbf*", "", 2)});
  REQUIRE(f.points.size() == 1);
  REQUIRE(f.shots.size() == 6);

  const int expected_hitters[] = {1, 2, 1, 2, 1, 2};
  for (std::size_t i = 0; i < 6; ++i) {
    CHECK(f.shots[i].hitter == expected_hitters[i]);
    CHECK(f.shots[i].shot_no == static_cast<int>(i) + 1);
  }
  CHECK(f.shots[0].is_serve);
  CHECK(f.shots[0].direction == 4);
  CHECK(f.shots[1].type == 'f');
  CHECK_FALSE(f.shots[4].is_last);
  CHECK(f.shots[5].is_last);
  CHECK(f.shots[5].ender == Ender::kWinner);

  const auto& p = f.points[0];
  CHECK(p.rally_len == 6);
  CHECK(p.last_hitter == 2);
  CHECK(p.implied_winner == 2);
  CHECK_FALSE(p.ace);
}

TEST_CASE("an ace is a one-shot point won by the server", "[ir]") {
  const auto f = flatten({row(1, 2, "6*", "", 2)});
  REQUIRE(f.points.size() == 1);
  CHECK(f.points[0].ace);
  CHECK(f.points[0].rally_len == 1);
  CHECK(f.points[0].implied_winner == 2);
  REQUIRE(f.shots.size() == 1);
  CHECK(f.shots[0].is_last);
}

TEST_CASE("a second serve point is read from the 2nd column", "[ir]") {
  const auto f = flatten({row(1, 1, "4w", "5f*", 2)});
  REQUIRE(f.points.size() == 1);
  const auto& p = f.points[0];
  CHECK(p.second_serve);
  CHECK(p.first_serve_fault);
  CHECK_FALSE(p.double_fault);
  CHECK(p.serve_dir == 5); // the second serve's direction, not the fault's
  REQUIRE(f.shots.size() == 2);
  CHECK(f.shots[0].second_serve);
}

TEST_CASE("two faults in a row is a double fault, returner wins", "[ir]") {
  const auto f = flatten({row(1, 1, "4w", "6d", 2)});
  REQUIRE(f.points.size() == 1);
  const auto& p = f.points[0];
  CHECK(p.double_fault);
  CHECK(p.implied_winner == 2);
  CHECK(p.rally_len == 1);
}

TEST_CASE("an error marker on a bare serve is the returner's error", "[ir]") {
  // "4#" - serve came in, return wasn't charted, returner couldn't get it back
  const auto f = flatten({row(1, 1, "4#", "", 1)});
  REQUIRE(f.points.size() == 1);
  const auto& p = f.points[0];
  CHECK(p.last_hitter == 2);
  CHECK(p.implied_winner == 1);
  CHECK_FALSE(p.ace);
  CHECK(p.ender == Ender::kForcedError);
}

TEST_CASE("an unforced error by the server hands the point to the returner", "[ir]") {
  // serve, return, then the server nets one: 3 shots, server hit the last
  const auto f = flatten({row(1, 1, "4fbn@", "", 2)});
  REQUIRE(f.points.size() == 1);
  CHECK(f.points[0].last_hitter == 1);
  CHECK(f.points[0].implied_winner == 2);
}

TEST_CASE("rows with nothing to parse are skipped", "[ir]") {
  const auto f = flatten({row(1, 1, "", "", 1), row(2, 1, "4*", "", 1)});
  CHECK(f.points.size() == 1);
  CHECK(f.points[0].pt == 2);
}

TEST_CASE("a shot row turns into a csv line", "[ir]") {
  const auto f = flatten({row(1, 2, "6*", "", 2)});
  REQUIRE(f.shots.size() == 1);
  CHECK(to_csv_line(f.shots[0]) == "m,1,1,2,serve,6,,,1,winner,,0,2,1");
}
