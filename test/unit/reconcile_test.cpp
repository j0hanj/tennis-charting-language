#include "sema/reconcile.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

using tcl::match::PointRow;
using tcl::sema::reconcile_score;

namespace {

PointRow row(std::string match_id, int pt, int server, std::string pts, int pt_winner,
             int line_no = 0) {
  PointRow r;
  r.match_id = std::move(match_id);
  r.pt = pt;
  r.server = server;
  r.pts = std::move(pts);
  r.pt_winner = pt_winner;
  r.line_no = line_no;
  return r;
}

}  // namespace

TEST_CASE("agrees when the file's score matches the replay", "[sema]") {
  const std::vector<PointRow> rows{
      row("m", 1, 1, "0-0", 1),
      row("m", 2, 1, "15-0", 1),
      row("m", 3, 1, "30-0", 2),
  };
  const auto r = reconcile_score(rows);
  CHECK(r.issues.empty());
  CHECK(r.points_checked == 3);
}

TEST_CASE("Pts is server-first, not player-1-first", "[sema]") {
  // player 2 is serving and wins the first point: internally that's
  // points = {0, 15}, but the file should read "15-0" (server's count first).
  const std::vector<PointRow> rows{
      row("m", 1, 2, "0-0", 2),
      row("m", 2, 2, "15-0", 1),
  };
  const auto r = reconcile_score(rows);
  CHECK(r.issues.empty());
}

TEST_CASE("flags a score that doesn't match, with what we expected", "[sema]") {
  const std::vector<PointRow> rows{
      row("m", 1, 1, "0-0", 1),
      row("m", 2, 1, "99-99", 1, 42), // wrong on purpose
  };
  const auto r = reconcile_score(rows);
  REQUIRE(r.issues.size() == 1);
  CHECK(r.issues[0].expected == "15-0");
  CHECK(r.issues[0].got == "99-99");
  CHECK(r.issues[0].line_no == 42);
}

TEST_CASE("case doesn't matter for AD-40 vs Ad-40", "[sema]") {
  const std::vector<PointRow> rows{
      // drive the game to deuce first (no pts check on these)
      row("m", 1, 1, "", 1), row("m", 2, 1, "", 1), row("m", 3, 1, "", 1),
      row("m", 4, 1, "", 2), row("m", 5, 1, "", 2), row("m", 6, 1, "", 2),
      row("m", 7, 1, "40-40", 1), // server takes the advantage point
      row("m", 8, 1, "AD-40", 2), // file writes it in caps, we render "Ad-40"
  };
  const auto r = reconcile_score(rows);
  CHECK(r.issues.empty());
}

TEST_CASE("a missing PtWinner stops that match without crashing", "[sema]") {
  const std::vector<PointRow> rows{
      row("m", 1, 1, "0-0", 0), // no winner recorded
      row("m", 2, 1, "15-0", 1),
  };
  const auto r = reconcile_score(rows);
  REQUIRE(r.issues.size() == 1);
  CHECK(r.issues[0].message.find("PtWinner") != std::string::npos);
}

TEST_CASE("a new match_id resets the score", "[sema]") {
  std::vector<PointRow> rows{
      row("m1", 1, 1, "0-0", 1),
      row("m2", 1, 1, "0-0", 1), // different match, should start over at 0-0
  };
  const auto r = reconcile_score(rows);
  CHECK(r.issues.empty());
}

TEST_CASE("rows past a finished match get one issue, not one per row", "[sema]") {
  std::vector<PointRow> rows;
  std::string set;
  // alternate games so nobody wins the set early
  auto win_game = [&](int winner) {
    for (int i = 0; i < 4; ++i) rows.push_back(row("m", 0, 1, "", winner));
  };
  // two 6-0 sets: P1 wins every game, straightforwardly finishes 2-0
  for (int s = 0; s < 2; ++s)
    for (int g = 0; g < 6; ++g) win_game(1);
  rows.push_back(row("m", 99, 1, "0-0", 1, 7)); // one row too many

  const auto r = reconcile_score(rows);
  REQUIRE(r.issues.size() == 1);
  CHECK(r.issues[0].message.find("finished") != std::string::npos);
}
