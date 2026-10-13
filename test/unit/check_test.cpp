#include "scoring/check.hpp"

#include <catch2/catch_test_macros.hpp>

using tcl::scoring::check_format;
using tcl::scoring::MatchFormat;

TEST_CASE("a plain advantage-set best of three has no violations", "[check]") {
  const auto r = check_format(MatchFormat::best_of_three_advantage_set());
  CHECK(r.ok());
  CHECK(r.violations.empty());
  CHECK(r.states_explored > 0);
}

TEST_CASE("a tiebreak format has no violations", "[check]") {
  const auto r = check_format(MatchFormat::best_of_three_with_tiebreak());
  CHECK(r.ok());
  CHECK(r.violations.empty());
}

TEST_CASE("nothing ever decreases except by resetting to 0", "[check]") {
  // every count climbs, resets, or stays put across the whole reachable
  // space - this is really just re-stating .ok(), but says what it's
  // actually testing for
  const auto r = check_format(MatchFormat::best_of_three_with_tiebreak());
  CHECK(r.monotonic);
}

TEST_CASE("the search actually hits its bound on a no-tiebreak format", "[check]") {
  // without a tiebreak, deuce can run forever - confirms the cap is doing
  // something rather than the BFS just finishing early on its own
  const auto r = check_format(MatchFormat::best_of_three_advantage_set());
  CHECK(r.states_bounded > 0);
}

TEST_CASE("the search actually reaches a finished state", "[check]") {
  const auto r = check_format(MatchFormat::best_of_three_with_tiebreak());
  CHECK(r.reaches_finish);
  CHECK(r.ok());
}

TEST_CASE("every format on the menu reaches a finished state within the bound", "[check]") {
  for (const auto& fmt : {MatchFormat::best_of_three_advantage_set(),
                          MatchFormat::best_of_three_with_tiebreak(),
                          MatchFormat::best_of_five_with_tiebreak(),
                          MatchFormat::best_of_three_with_final_set_breaker(),
                          MatchFormat::best_of_five_with_final_set_breaker(),
                          MatchFormat::best_of_three_with_match_tiebreak_decider(),
                          MatchFormat::nextgen_finals()}) {
    const auto r = check_format(fmt);
    CHECK(r.ok());
  }
}
