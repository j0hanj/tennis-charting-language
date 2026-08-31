// Placeholder test so CI has something green to run before the library exists.
// Replace with real lexer/parser/scoring tests as those modules land.

#include <catch2/catch_test_macros.hpp>

TEST_CASE("build and test harness works", "[smoke]") {
  REQUIRE(1 + 1 == 2);
}
