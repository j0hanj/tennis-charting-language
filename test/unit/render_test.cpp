#include "lexer/render.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using tcl::lexer::Diagnostic;
using tcl::lexer::render_diagnostic;
using tcl::lexer::render_diagnostics;

TEST_CASE("caret sits under the offending character", "[render]") {
  const std::string out = render_diagnostic("4fQf*", {2, 1, "don't recognize 'Q'"});
  CHECK(out == "  4fQf*\n    ^ don't recognize 'Q'");
}

TEST_CASE("length widens the caret run", "[render]") {
  const std::string out = render_diagnostic("4ffbb", {1, 3, "huh"});
  CHECK(out == "  4ffbb\n   ^^^ huh");
}

TEST_CASE("an offset past the end still points somewhere", "[render]") {
  const std::string out = render_diagnostic("4ff", {99, 1, "expected * @ or #"});
  // caret lands right after the last character
  CHECK(out == "  4ff\n     ^ expected * @ or #");
}

TEST_CASE("a batch prints the source once", "[render]") {
  std::vector<Diagnostic> ds{{0, 1, "no serve direction"}, {3, 1, "trailing junk"}};
  const std::string out = render_diagnostics("ff*x", ds);
  CHECK(out ==
        "  ff*x\n"
        "  ^ no serve direction\n"
        "     ^ trailing junk");
}
