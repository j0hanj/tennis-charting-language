#include "lexer/lexer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using tcl::lexer::Diagnostic;
using tcl::lexer::Kind;
using tcl::lexer::kind_name;
using tcl::lexer::lex;
using tcl::lexer::Token;

namespace {

// Kinds of every token except the trailing kEnd.
std::vector<Kind> kinds(std::string_view src) {
  auto lexed = lex(src);
  std::vector<Kind> ks;
  for (const auto& t : lexed.tokens) {
    if (t.kind != Kind::kEnd) ks.push_back(t.kind);
  }
  return ks;
}

}  // namespace

TEST_CASE("empty string is just an end token", "[lexer]") {
  auto lexed = lex("");
  REQUIRE(lexed.tokens.size() == 1);
  CHECK(lexed.tokens[0].kind == Kind::kEnd);
  CHECK(lexed.tokens[0].offset == 0);
  CHECK(lexed.ok());
}

TEST_CASE("a plain point tokenizes cleanly", "[lexer]") {
  auto lexed = lex("4ffbbf*");
  CHECK(lexed.ok());
  CHECK(kinds("4ffbbf*") == std::vector<Kind>{
      Kind::kDigit, Kind::kShotType, Kind::kShotType, Kind::kShotType,
      Kind::kShotType, Kind::kShotType, Kind::kEndMarker});
  CHECK(lexed.tokens.back().kind == Kind::kEnd);
  CHECK(lexed.tokens.back().offset == 7);
}

TEST_CASE("digit tokens carry their value", "[lexer]") {
  auto lexed = lex("459");
  REQUIRE(lexed.tokens.size() == 4);  // 3 digits + end
  CHECK(lexed.tokens[0].value == 4);
  CHECK(lexed.tokens[1].value == 5);
  CHECK(lexed.tokens[2].value == 9);
}

TEST_CASE("offsets point back at the source", "[lexer]") {
  auto lexed = lex("4f*");
  CHECK(lexed.tokens[0].offset == 0);
  CHECK(lexed.tokens[1].offset == 1);
  CHECK(lexed.tokens[2].offset == 2);
  CHECK(lexed.tokens[1].text == "f");
}

TEST_CASE("positions, end markers and error locations", "[lexer]") {
  CHECK(kinds("+-=") == std::vector<Kind>{Kind::kPosition, Kind::kPosition, Kind::kPosition});
  CHECK(kinds("*@#") == std::vector<Kind>{Kind::kEndMarker, Kind::kEndMarker, Kind::kEndMarker});
  CHECK(kinds("@n") == std::vector<Kind>{Kind::kEndMarker, Kind::kErrorLoc});
  CHECK(kinds("#w") == std::vector<Kind>{Kind::kEndMarker, Kind::kErrorLoc});
}

TEST_CASE("whitespace is ignored", "[lexer]") {
  CHECK(kinds("4 f f *") == std::vector<Kind>{
      Kind::kDigit, Kind::kShotType, Kind::kShotType, Kind::kEndMarker});
}

TEST_CASE("an unknown character is flagged but scanning continues", "[lexer]") {
  auto lexed = lex("4fQf*");
  CHECK_FALSE(lexed.ok());
  REQUIRE(lexed.diagnostics.size() == 1);
  CHECK(lexed.diagnostics[0].offset == 2);

  // the tokens after the bad character are still there
  CHECK(kinds("4fQf*") == std::vector<Kind>{
      Kind::kDigit, Kind::kShotType, Kind::kUnknown, Kind::kShotType, Kind::kEndMarker});
}

TEST_CASE("two unknown characters give two diagnostics", "[lexer]") {
  auto lexed = lex("Q4fZ");
  REQUIRE(lexed.diagnostics.size() == 2);
  CHECK(lexed.diagnostics[0].offset == 0);
  CHECK(lexed.diagnostics[1].offset == 3);
}

TEST_CASE("a longer rally from the notation doc scans clean", "[lexer]") {
  auto lexed = lex("5r37b+3l2o=1r#");
  CHECK(lexed.ok());
  CHECK(lexed.tokens.size() == 15);  // 14 chars + end
}

TEST_CASE("kind_name covers every kind", "[lexer]") {
  for (auto k : {Kind::kShotType, Kind::kDigit, Kind::kPosition, Kind::kEndMarker,
                 Kind::kErrorLoc, Kind::kUnknown, Kind::kEnd}) {
    CHECK(std::string(kind_name(k)) != "?");
  }
}
