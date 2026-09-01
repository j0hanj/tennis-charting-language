// Tennis scoring state machine.
//
// Scope so far: best-of-three sets, either plain advantage sets (default) or
// with a standard 7-point tiebreak at 6-6. Best-of-five and the per-tournament
// final-set rules come later, still through MatchFormat rather than branching.

#ifndef TCL_SCORING_SCORE_HPP
#define TCL_SCORING_SCORE_HPP

#include <array>
#include <cstdint>
#include <string>

namespace tcl::scoring {

enum class Player : std::uint8_t { kOne = 0, kTwo = 1 };

constexpr Player other(Player p) {
  return p == Player::kOne ? Player::kTwo : Player::kOne;
}

constexpr int index(Player p) { return static_cast<int>(p); }

// Rules that vary between competitions. Only the pieces needed today are
// modelled; fields are added as formats are supported.
struct MatchFormat {
  int sets_to_win = 2;        // best-of-three
  int games_to_win_set = 6;   // first to six games...
  int set_win_margin = 2;     // ...winning by two, otherwise the set continues
  int points_to_win_game = 4; // 0/15/30/40 then a fourth point...
  int game_win_margin = 2;    // ...winning by two (deuce / advantage)

  bool has_tiebreak = false;      // if false, a set just keeps going past 6-6
  int games_for_tiebreak = 6;     // play a breaker once both hit this many games
  int tiebreak_points_to_win = 7; // first to seven points...
  int tiebreak_win_margin = 2;    // ...winning by two. winning it wins the set 7-6.

  static constexpr MatchFormat best_of_three_advantage_set() { return {}; }

  static constexpr MatchFormat best_of_three_with_tiebreak() {
    MatchFormat f;
    f.has_tiebreak = true;
    return f;
  }
};

// A complete point-in-time state of a match.
struct Score {
  std::array<int, 2> points{0, 0}; // raw points won in the current game
  std::array<int, 2> games{0, 0};  // games won in the current set
  std::array<int, 2> sets{0, 0};   // sets won in the match
  Player server = Player::kOne;    // who is serving the current game
  bool finished = false;
  Player winner = Player::kOne; // meaningful only when finished

  friend bool operator==(const Score&, const Score&) = default;
};

// Advance the match by one point won by `point_winner`.
// Precondition: !score.finished. Returns the resulting state.
Score step(const Score& score, Player point_winner,
           const MatchFormat& fmt = MatchFormat::best_of_three_advantage_set());

// Human-readable current game score, e.g. "0-0", "40-30", "40-40",
// "Ad-40" (server has advantage), "40-Ad". During a tiebreak this is just the
// raw point count instead, e.g. "5-3". Games/sets not included.
std::string game_score(const Score& score,
                        const MatchFormat& fmt = MatchFormat::best_of_three_advantage_set());

// Broadcast-style line: "1-0 4-3 40-30" (sets, games, points). Once the match is
// over only the set score is left, e.g. "2-1" (we don't keep per-set history).
std::string scoreline(const Score& score,
                       const MatchFormat& fmt = MatchFormat::best_of_three_advantage_set());

// One-line summary: "sets 1-0 | games 4-3 | 40-30 | server: P1".
std::string describe(const Score& score,
                      const MatchFormat& fmt = MatchFormat::best_of_three_advantage_set());

}  // namespace tcl::scoring

#endif  // TCL_SCORING_SCORE_HPP
