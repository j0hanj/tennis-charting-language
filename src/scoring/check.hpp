// A small model checker for the scoring engine: BFS every state reachable
// from 0-0 under a given MatchFormat and check it never does anything a real
// tennis match couldn't.
//
// The raw point count can climb forever through an unbroken string of deuces
// in a no-tiebreak format, so an unbounded search would never finish. This
// bounds how far past a winning target it looks (see point_cap/game_cap in
// check.cpp) - past that it just stops expanding that branch rather than
// claiming anything about it. States within the bound are checked completely.

#ifndef TCL_SCORING_CHECK_HPP
#define TCL_SCORING_CHECK_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "scoring/score.hpp"

namespace tcl::scoring {

struct CheckResult {
  std::size_t states_explored = 0;
  std::size_t states_bounded = 0; // successors not expanded because they hit the cap
  bool all_valid = true;          // every explored state passed validate()
  bool monotonic = true;          // no transition decreased a count except by resetting to 0
  std::vector<std::string> violations; // human-readable, first handful

  bool ok() const { return all_valid && monotonic; }
};

// Explore every state reachable from Score{} under `fmt`. Checks that no
// state is structurally impossible (see validate() in check.cpp) and that no
// transition between two states decreases a player's point/game/set count
// except by resetting it to exactly 0 (a game, set, or match being won).
CheckResult check_format(const MatchFormat& fmt);

}  // namespace tcl::scoring

#endif  // TCL_SCORING_CHECK_HPP
