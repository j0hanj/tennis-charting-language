#include "scoring/score.hpp"

#include <array>
#include <cassert>

namespace tcl::scoring {

namespace {

const char* point_label(int p) {
  static constexpr std::array<const char*, 4> kLabels{"0", "15", "30", "40"};
  if (p < 0) return "0";
  if (p >= static_cast<int>(kLabels.size())) return "40";
  return kLabels[static_cast<std::size_t>(p)];
}

}  // namespace

Score step(const Score& score, Player point_winner, const MatchFormat& fmt) {
  assert(!score.finished && "step() called on a finished match");

  Score s = score;
  const int w = index(point_winner);
  const int l = index(other(point_winner));

  s.points[w] += 1;

  const bool game_won = s.points[w] >= fmt.points_to_win_game &&
                        s.points[w] - s.points[l] >= fmt.game_win_margin;
  if (!game_won) {
    return s;
  }

  s.points = {0, 0};
  s.games[w] += 1;
  s.server = other(s.server);

  const bool set_won = s.games[w] >= fmt.games_to_win_set &&
                       s.games[w] - s.games[l] >= fmt.set_win_margin;
  if (!set_won) {
    return s;
  }

  s.games = {0, 0};
  s.sets[w] += 1;

  if (s.sets[w] >= fmt.sets_to_win) {
    s.finished = true;
    s.winner = point_winner;
  }
  return s;
}

std::string game_score(const Score& score) {
  const int a = score.points[0];
  const int b = score.points[1];

  if (a >= 3 && b >= 3) {
    if (a == b) return "40-40";
    return a > b ? "Ad-40" : "40-Ad";
  }
  return std::string(point_label(a)) + "-" + point_label(b);
}

std::string scoreline(const Score& score) {
  std::string s = std::to_string(score.sets[0]) + "-" + std::to_string(score.sets[1]);
  if (score.finished) {
    return s;
  }
  s += " " + std::to_string(score.games[0]) + "-" + std::to_string(score.games[1]);
  s += " " + game_score(score);
  return s;
}

std::string describe(const Score& score) {
  if (score.finished) {
    return "FINISHED | sets " + std::to_string(score.sets[0]) + "-" +
           std::to_string(score.sets[1]) + " | winner: " +
           (score.winner == Player::kOne ? "P1" : "P2");
  }
  return "sets " + std::to_string(score.sets[0]) + "-" + std::to_string(score.sets[1]) +
         " | games " + std::to_string(score.games[0]) + "-" + std::to_string(score.games[1]) +
         " | " + game_score(score) + " | server: " +
         (score.server == Player::kOne ? "P1" : "P2");
}

}  // namespace tcl::scoring
