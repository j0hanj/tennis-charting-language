#include "scoring/score.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <vector>

using tcl::scoring::describe;
using tcl::scoring::game_score;
using tcl::scoring::scoreline;
using tcl::scoring::Player;
using tcl::scoring::Score;
using tcl::scoring::step;

namespace {

// Play a sequence of points, "A" = player one wins, "B" = player two wins.
Score play(const std::string& points, Score s = {}) {
  for (const char c : points) {
    s = step(s, c == 'A' ? Player::kOne : Player::kTwo);
  }
  return s;
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
