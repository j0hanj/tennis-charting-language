#include "scoring/check.hpp"

#include <catch2/catch_test_macros.hpp>

using tcl::scoring::check_format;
using tcl::scoring::MatchFormat;

TEST_CASE("a plain advantage-set best of three has no violations", "[check]") {
  const auto r = check_format(MatchFormat::best_of_three_advantage_set());
  CHECK(r.all_valid);
  CHECK(r.violations.empty());
  CHECK(r.states_explored > 0);
}

TEST_CASE("a tiebreak format has no violations", "[check]") {
  const auto r = check_format(MatchFormat::best_of_three_with_tiebreak());
  CHECK(r.all_valid);
  CHECK(r.violations.empty());
}

TEST_CASE("the search actually hits its bound on a no-tiebreak format", "[check]") {
  // without a tiebreak, deuce can run forever - confirms the cap is doing
  // something rather than the BFS just finishing early on its own
  const auto r = check_format(MatchFormat::best_of_three_advantage_set());
  CHECK(r.states_bounded > 0);
}
