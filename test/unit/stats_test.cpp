#include "analytics/stats.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "sema/outcomes.hpp"

using tcl::analytics::compute_stats;
using tcl::analytics::format_report;
using tcl::analytics::names_from_match_id;
using tcl::analytics::split_by_match;
using tcl::ir::flatten;
using tcl::match::PointRow;

namespace {

PointRow row(std::string match_id, int pt, int server, std::string first, std::string second,
             int winner) {
  PointRow r;
  r.match_id = std::move(match_id);
  r.pt = pt;
  r.server = server;
  r.first = std::move(first);
  r.second = std::move(second);
  r.pt_winner = winner;
  return r;
}

// four points, small enough to check every number by hand:
//   1  P1 serves, ace                              -> P1
//   2  P1 serves, double fault                     -> P2
//   3  P2 serves, P1 hits a return winner          -> P1
//   4  P1 serves, then P1 nets a shot (unforced)   -> P2
std::vector<PointRow> four_points() {
  return {
      row("m", 1, 1, "4*", "", 1),
      row("m", 2, 1, "4w", "6d", 2),
      row("m", 3, 2, "5f*", "", 1),
      row("m", 4, 1, "6fbn@", "", 2),
  };
}

}  // namespace

TEST_CASE("player names come out of the match id", "[stats]") {
  const auto n = names_from_match_id("20260521-M-Roland_Garros-Q3-Jesper_De_Jong-Michael_Zheng");
  CHECK(n[0] == "Jesper De Jong");
  CHECK(n[1] == "Michael Zheng");

  const auto fallback = names_from_match_id("something-else");
  CHECK(fallback[0] == "P1");
  CHECK(fallback[1] == "P2");
}

TEST_CASE("serve and ending numbers add up", "[stats]") {
  const auto st = compute_stats(flatten(four_points()).points);

  CHECK(st.points == 4);
  CHECK(st.player[0].points_won == 2);
  CHECK(st.player[1].points_won == 2);

  // P1 served points 1, 2 and 4
  CHECK(st.player[0].service_points == 3);
  CHECK(st.player[0].service_points_won == 1);
  CHECK(st.player[0].first_serves_in == 2); // missed the first serve on point 2
  CHECK(st.player[0].aces == 1);
  CHECK(st.player[0].double_faults == 1);
  CHECK(st.player[0].unforced == 1);
  CHECK(st.player[0].winners == 1); // the return winner on point 3, not the ace

  CHECK(st.player[1].service_points == 1);
  CHECK(st.player[1].service_points_won == 0);
}

TEST_CASE("rally lengths skip double faults", "[stats]") {
  const auto st = compute_stats(flatten(four_points()).points);
  CHECK(st.rally_hist.size() == 3); // lengths 1, 2 and 3
  CHECK(st.rally_hist.at(1) == 1);
  CHECK(st.rally_hist.at(2) == 1);
  CHECK(st.rally_hist.at(3) == 1);
  CHECK(st.longest == 3);
  CHECK(st.avg_rally == 2.0);
  CHECK(st.by_length[0].points == 3); // all three fit in "1-3"
  CHECK(st.by_length[0].unforced == 1);
}

TEST_CASE("serve direction tracks how the server did", "[stats]") {
  const auto st = compute_stats(flatten(four_points()).points);
  CHECK(st.serve_dir[0].points == 1); // wide (4): the ace
  CHECK(st.serve_dir[0].won == 1);
  CHECK(st.serve_dir[0].aces == 1);
  CHECK(st.serve_dir[1].points == 1); // body (5): P2's serve, P1 won it
  CHECK(st.serve_dir[1].won == 0);
  CHECK(st.serve_dir[2].points == 1); // T (6): P1 lost it
  CHECK(st.serve_dir[2].won == 0);
}

TEST_CASE("a point where the string and PtWinner disagree is counted", "[stats]") {
  // P1's ace, but the file says P2 won it
  const auto st = compute_stats(flatten({row("m", 1, 1, "4*", "", 2)}).points);
  CHECK(st.disagreements == 1);
  CHECK(compute_stats(flatten(four_points()).points).disagreements == 0);
}

TEST_CASE("a file with two matches splits in file order", "[stats]") {
  const auto pts = flatten({row("a", 1, 1, "4*", "", 1), row("a", 2, 1, "4*", "", 1),
                            row("b", 1, 1, "4*", "", 1)})
                       .points;
  const auto groups = split_by_match(pts);
  REQUIRE(groups.size() == 2);
  CHECK(groups[0].size() == 2);
  CHECK(groups[1].size() == 1);
  CHECK(groups[1][0].match_id == "b");
}

TEST_CASE("the report has the sections and the names", "[stats]") {
  const std::string report = format_report(compute_stats(flatten(four_points()).points));
  CHECK(report.find("(4 points)") != std::string::npos);
  CHECK(report.find("won on serve") != std::string::npos);
  CHECK(report.find("rally length") != std::string::npos);
  CHECK(report.find("how points end") != std::string::npos);
  CHECK(report.find("serve direction") != std::string::npos);
}

TEST_CASE("an empty match is just an empty report, not a crash", "[stats]") {
  const auto st = compute_stats({});
  CHECK(st.points == 0);
  CHECK(format_report(st).find("(0 points)") != std::string::npos);
}

TEST_CASE("check_outcomes flags a shot string that contradicts PtWinner", "[stats]") {
  using tcl::sema::check_outcomes;

  // "4ffbbf*": serve + 5 shots, the returner hits the last one and wins
  CHECK(check_outcomes(flatten({row("m", 1, 1, "4ffbbf*", "", 2)}).points).empty());

  const auto wrong = check_outcomes(flatten({row("m", 1, 1, "4ffbbf*", "", 1)}).points);
  REQUIRE(wrong.size() == 1);
  CHECK(wrong[0].expected == "2");
  CHECK(wrong[0].got == "1");

  // a double fault always goes to the returner
  CHECK(check_outcomes(flatten({row("m", 1, 1, "4w", "6d", 2)}).points).empty());
  CHECK(check_outcomes(flatten({row("m", 1, 1, "4w", "6d", 1)}).points).size() == 1);

  // no ending marker - the string can't say who won, so nothing to check
  CHECK(check_outcomes(flatten({row("m", 1, 1, "4ff", "", 1)}).points).empty());
}
