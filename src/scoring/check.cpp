#include "scoring/check.hpp"

#include <queue>
#include <set>
#include <tuple>

namespace tcl::scoring {

namespace {

using StateKey = std::tuple<int, int, int, int, int, int, int, bool, int>;

StateKey key_of(const Score& s) {
  return {s.points[0], s.points[1], s.games[0],  s.games[1],
          s.sets[0],   s.sets[1],   index(s.server), s.finished, index(s.winner)};
}

// How far past a winning target the search follows a branch before giving up
// on it. Points can climb forever through an unbroken string of deuces in a
// no-tiebreak format, so this has to stop somewhere - +4 is enough to see a
// game/set won outright, won from deuce, and won from deuce after trading
// the advantage back once, which is as far as the rules actually differ.
int point_cap(const MatchFormat& fmt) {
  int target = fmt.points_to_win_game;
  if (fmt.tiebreak_points_to_win > target) target = fmt.tiebreak_points_to_win;
  if (fmt.final_set_tiebreak_to > target) target = fmt.final_set_tiebreak_to;
  return target + 4;
}

int game_cap(const MatchFormat& fmt) { return fmt.games_to_win_set + 4; }

bool game_score_already_over(int w, int l, const MatchFormat& fmt) {
  return w >= fmt.points_to_win_game && w - l >= fmt.game_win_margin;
}

bool set_score_already_over(int w, int l, const MatchFormat& fmt) {
  return w >= fmt.games_to_win_set && w - l >= fmt.set_win_margin;
}

// Structural checks only - not "did a real sequence of points reach this"
// (the BFS itself already guarantees that, since it only ever looks at
// states step() actually produced), just "could this ever legitimately be a
// snapshot of a match".
bool validate(const Score& s, const MatchFormat& fmt, std::vector<std::string>& violations) {
  bool ok = true;
  auto fail = [&](const char* why) {
    ok = false;
    if (violations.size() < 8) violations.emplace_back(why);
  };

  for (int i : {0, 1}) {
    if (s.points[i] < 0 || s.games[i] < 0 || s.sets[i] < 0) fail("negative count");
    if (s.sets[i] > fmt.sets_to_win) fail("sets exceed sets_to_win");
  }
  if (s.sets[0] >= fmt.sets_to_win && s.sets[1] >= fmt.sets_to_win) {
    fail("both players at sets_to_win");
  }
  if (s.finished) {
    const int w = index(s.winner);
    const int l = index(other(s.winner));
    if (s.sets[w] < fmt.sets_to_win) fail("finished but the winner hasn't reached sets_to_win");
    if (s.sets[l] >= fmt.sets_to_win) fail("finished but the loser also reached sets_to_win");
  } else if (s.sets[0] >= fmt.sets_to_win || s.sets[1] >= fmt.sets_to_win) {
    fail("sets_to_win reached but not marked finished");
  }

  // right at the breaker threshold, points follow different (higher) rules
  // this checker doesn't know about - see the header comment - so the normal
  // "game score already past the win condition" check is skipped there
  // rather than risk a false positive.
  const bool maybe_in_breaker = fmt.has_tiebreak && s.games[0] == fmt.games_for_tiebreak &&
                                s.games[1] == fmt.games_for_tiebreak;
  if (!maybe_in_breaker && (game_score_already_over(s.points[0], s.points[1], fmt) ||
                            game_score_already_over(s.points[1], s.points[0], fmt))) {
    fail("game score already past the win condition, should've reset");
  }
  if (set_score_already_over(s.games[0], s.games[1], fmt) ||
      set_score_already_over(s.games[1], s.games[0], fmt)) {
    fail("set score already past the win condition, should've reset");
  }
  return ok;
}

// No transition should ever decrease a count except by resetting it to
// exactly 0 - that covers a game/set/match being won in one check, without
// needing to know which kind of win it was.
bool transition_ok(const Score& a, const Score& b, std::vector<std::string>& violations) {
  bool ok = true;
  auto fail = [&](const char* why) {
    ok = false;
    if (violations.size() < 8) violations.emplace_back(why);
  };
  for (int i : {0, 1}) {
    if (b.sets[i] < a.sets[i]) fail("a transition decreased a set count");
    if (b.games[i] < a.games[i] && b.games[i] != 0) {
      fail("a transition decreased a game count without resetting to 0");
    }
    if (b.points[i] < a.points[i] && b.points[i] != 0) {
      fail("a transition decreased a point count without resetting to 0");
    }
  }
  return ok;
}

}  // namespace

CheckResult check_format(const MatchFormat& fmt) {
  CheckResult out;
  const int pcap = point_cap(fmt);
  const int gcap = game_cap(fmt);

  std::set<StateKey> visited;
  std::queue<Score> frontier;
  const Score start;
  visited.insert(key_of(start));
  frontier.push(start);

  while (!frontier.empty()) {
    const Score s = frontier.front();
    frontier.pop();
    ++out.states_explored;

    if (!validate(s, fmt, out.violations)) out.all_valid = false;
    if (s.finished) continue; // no outgoing transitions from a finished match

    for (const Player p : {Player::kOne, Player::kTwo}) {
      const Score next = step(s, p, fmt);
      if (!transition_ok(s, next, out.violations)) out.monotonic = false;
      if (next.points[0] > pcap || next.points[1] > pcap || next.games[0] > gcap ||
          next.games[1] > gcap) {
        ++out.states_bounded;
        continue;
      }
      if (visited.insert(key_of(next)).second) frontier.push(next);
    }
  }
  return out;
}

}  // namespace tcl::scoring
