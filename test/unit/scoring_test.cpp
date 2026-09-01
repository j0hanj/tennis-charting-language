#include "scoring/score.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>

using tcl::scoring::describe;
using tcl::scoring::game_score;
using tcl::scoring::MatchFormat;
using tcl::scoring::scoreline;
using tcl::scoring::Player;
using tcl::scoring::Score;
using tcl::scoring::step;

namespace {

// Play a sequence of points, "A" = player one wins, "B" = player two wins.
Score play(const std::string& points, Score s = {},
           const MatchFormat& fmt = MatchFormat::best_of_three_advantage_set()) {
  for (const char c : points) {
    s = step(s, c == 'A' ? Player::kOne : Player::kTwo, fmt);
  }
  return s;
}

// Alternating games so the set reaches 6-6 without either player getting the
// two-game lead that would win it earlier.
std::string games_to_six_all() {
  std::string p;
  for (int i = 0; i < 6; ++i) p += "AAAABBBB";
  return p;
}

}  // namespace

TEST_CASE("game score labels", "[scoring]") {
  Score s;
  CHECK(game_score(s) == "0-0");
  s = play("A");
  CHECK(game_score(s) == "15-0");
  s = play("AB");
  CHECK(game_score(s) == "15-15");
  s = play("AAAB");
  CHECK(game_score(s) == "40-15");
}

TEST_CASE("a love game is won in four points", "[scoring]") {
  const Score s = play("AAAA");
  CHECK(s.games[0] == 1);
  CHECK(s.games[1] == 0);
  CHECK(s.points == std::array<int, 2>{0, 0});
}

TEST_CASE("service alternates every game", "[scoring]") {
  Score s;
  CHECK(s.server == Player::kOne);
  s = play("AAAA");
  CHECK(s.server == Player::kTwo);
  s = play("BBBB", s);
  CHECK(s.server == Player::kOne);
}

TEST_CASE("deuce requires winning by two", "[scoring]") {
  Score s = play("AAABBB");  // 40-40
  CHECK(game_score(s) == "40-40");
  CHECK(s.games == std::array<int, 2>{0, 0});

  s = step(s, Player::kOne);  // advantage P1
  CHECK(game_score(s) == "Ad-40");

  s = step(s, Player::kTwo);  // back to deuce
  CHECK(game_score(s) == "40-40");

  s = step(s, Player::kTwo);  // advantage P2
  s = step(s, Player::kTwo);  // P2 takes the game
  CHECK(s.games == std::array<int, 2>{0, 1});
}

TEST_CASE("a set is won at 6-4", "[scoring]") {
  std::string points;
  for (int i = 0; i < 6; ++i) points += "AAAA";  // P1 holds/breaks to 6-0
  Score s = play(points);
  // that already won the set 6-0
  CHECK(s.sets == std::array<int, 2>{1, 0});
  CHECK(s.games == std::array<int, 2>{0, 0});
}

TEST_CASE("no tiebreak: 5-5 set continues to 7-5", "[scoring]") {
  std::string points;
  for (int i = 0; i < 5; ++i) points += "AAAA";  // 5-0
  for (int i = 0; i < 5; ++i) points += "BBBB";  // 5-5
  Score s = play(points);
  CHECK(s.games == std::array<int, 2>{5, 5});
  CHECK(s.sets == std::array<int, 2>{0, 0});

  s = play("AAAA", s);  // 6-5, set not over
  CHECK(s.games == std::array<int, 2>{6, 5});
  CHECK(s.sets == std::array<int, 2>{0, 0});

  s = play("AAAA", s);  // 7-5, set won
  CHECK(s.sets == std::array<int, 2>{1, 0});
}

TEST_CASE("match ends after two sets and reports the winner", "[scoring]") {
  std::string set;
  for (int i = 0; i < 6; ++i) set += "AAAA";  // one 6-0 set for P1
  Score s = play(set + set);
  CHECK(s.finished);
  CHECK(s.winner == Player::kOne);
  CHECK(s.sets == std::array<int, 2>{2, 0});
  CHECK(describe(s) == "FINISHED | sets 2-0 | winner: P1");
}

TEST_CASE("scoreline reads like a broadcast graphic", "[scoring]") {
  CHECK(scoreline(Score{}) == "0-0 0-0 0-0");
  CHECK(scoreline(play("AAABB")) == "0-0 0-0 40-30");

  std::string set;
  for (int i = 0; i < 6; ++i) set += "AAAA";
  Score s = play(set + set);  // match over, P1 wins 2-0
  CHECK(s.finished);
  CHECK(scoreline(s) == "2-0");  // just the set score once it's done
}

TEST_CASE("tiebreak kicks in at 6-6 when the format has one", "[scoring][tiebreak]") {
  const auto fmt = MatchFormat::best_of_three_with_tiebreak();
  Score s = play(games_to_six_all(), {}, fmt);
  CHECK(s.games == std::array<int, 2>{6, 6});
  CHECK(s.sets == std::array<int, 2>{0, 0});

  // in the breaker now, raw point counts instead of 0/15/30/40
  s = play("AB", s, fmt);
  CHECK(game_score(s, fmt) == "1-1");

  s = play("AAAAAA", s, fmt); // P1 to 7 points, up by more than 2 already
  CHECK(s.games == std::array<int, 2>{0, 0}); // games reset, set was won
  CHECK(s.sets == std::array<int, 2>{1, 0});  // 7-6 set, no games-margin needed
}

TEST_CASE("tiebreak also needs to be won by two", "[scoring][tiebreak]") {
  const auto fmt = MatchFormat::best_of_three_with_tiebreak();
  Score s = play(games_to_six_all(), {}, fmt);

  s = play("ABABABABABAB", s, fmt); // 6-6 in the breaker
  CHECK(game_score(s, fmt) == "6-6");
  CHECK(s.sets == std::array<int, 2>{0, 0});

  s = step(s, Player::kOne, fmt); // 7-6, not enough
  CHECK(s.sets == std::array<int, 2>{0, 0});

  s = step(s, Player::kTwo, fmt); // back to 7-7
  s = step(s, Player::kOne, fmt); // 8-7
  s = step(s, Player::kOne, fmt); // 9-7, P1 wins the breaker
  CHECK(s.sets == std::array<int, 2>{1, 0});
}

TEST_CASE("without has_tiebreak the default still just keeps playing", "[scoring][tiebreak]") {
  // regression: adding tiebreak support shouldn't change the default format
  Score s = play(games_to_six_all()); // default, no tiebreak
  CHECK(s.games == std::array<int, 2>{6, 6});
  s = play("AA", s);
  CHECK(game_score(s) == "30-0"); // normal points, not a breaker
}

TEST_CASE("step never produces an invalid state over a long random-ish sequence", "[scoring]") {
  Score s;
  // Deterministic pseudo-pattern; enough games to finish a best-of-three.
  const std::string pattern = "ABBABAABBAABABBAABABABBABAABBA";
  int guard = 0;
  while (!s.finished && guard < 5000) {
    s = step(s, pattern[static_cast<std::size_t>(guard) % pattern.size()] == 'A'
                    ? Player::kOne
                    : Player::kTwo);
    for (int p : {0, 1}) {
      CHECK(s.points[p] >= 0);
      CHECK(s.games[p] >= 0);
      CHECK(s.sets[p] <= 2);
    }
    ++guard;
  }
  CHECK(s.finished);
}
