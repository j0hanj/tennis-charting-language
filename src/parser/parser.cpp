#include "parser/parser.hpp"

#include <string>

#include "lexer/lexer.hpp"

namespace tcl::parser {

namespace {

using tcl::ast::Ender;
using tcl::ast::ErrorLoc;
using tcl::ast::Point;
using tcl::ast::Shot;
using tcl::lexer::Diagnostic;
using tcl::lexer::Kind;
using tcl::lexer::Token;

// Walks the token stream. kUnknown tokens are dropped up front (the lexer
// already reported them), so the parser only ever sees things it understands.
class Cursor {
 public:
  Cursor(std::vector<Token> tokens, std::vector<Diagnostic>& diags)
      : tokens_(std::move(tokens)), diags_(diags) {}

  const Token& peek() const { return tokens_[pos_]; }
  const Token& advance() { return tokens_[pos_++]; }
  bool at(Kind k) const { return peek().kind == k; }
  bool done() const { return at(Kind::kEnd); }

  void error(std::size_t offset, std::string message) {
    diags_.push_back({offset, 1, std::move(message)});
  }

 private:
  std::vector<Token> tokens_;
  std::size_t pos_ = 0;
  std::vector<Diagnostic>& diags_;
};

// SHOTTYPE then any run of location bits (direction digit, depth digit, position
// marker) in whatever order they show up.
Shot parse_shot(Cursor& c) {
  const Token& head = c.advance();
  Shot shot;
  shot.type = head.text.empty() ? '?' : head.text[0];
  shot.offset = head.offset;

  while (c.at(Kind::kDigit) || c.at(Kind::kPosition)) {
    const Token& t = c.advance();
    if (t.kind == Kind::kPosition) {
      shot.position = t.text[0];
    } else if (t.value >= 1 && t.value <= 3 && !shot.direction) {
      shot.direction = t.value;
    } else if (t.value >= 7 && t.value <= 9 && !shot.depth) {
      shot.depth = t.value;
    } else if (!shot.direction) {
      shot.direction = t.value; // unusual digit, keep it as direction
    } else if (!shot.depth) {
      shot.depth = t.value;
    } else {
      c.error(t.offset, "extra digit on a shot");
    }
  }
  return shot;
}

std::optional<tcl::ast::Outcome> parse_ending(Cursor& c) {
  // real charted points put the error location right before the marker
  // ("6f18f3d@", not "...@d" like the doc examples I wrote first suggested -
  // found by running this against actual match charting project rows)
  std::optional<ErrorLoc> where;
  if (c.at(Kind::kErrorLoc)) {
    const Token& loc = c.advance();
    switch (loc.text[0]) {
      case 'n': where = ErrorLoc::kNet; break;
      case 'w': where = ErrorLoc::kWide; break;
      case 'd': where = ErrorLoc::kDeep; break;
      case 'x': where = ErrorLoc::kWideDeep; break;
      default: break;
    }
  }

  if (!c.at(Kind::kEndMarker)) {
    c.error(c.peek().offset, "point doesn't end with * @ or #");
    return std::nullopt;
  }

  const Token& marker = c.advance();
  tcl::ast::Outcome out;
  out.offset = marker.offset;
  out.where = where;
  switch (marker.text[0]) {
    case '*': out.ender = Ender::kWinner; break;
    case '@': out.ender = Ender::kUnforcedError; break;
    case '#': out.ender = Ender::kForcedError; break;
    default: break;
  }
  return out;
}

}  // namespace

Parsed parse(std::string_view src) {
  Parsed result;

  const auto lexed = tcl::lexer::lex(src);
  result.diagnostics = lexed.diagnostics;

  std::vector<Token> tokens;
  tokens.reserve(lexed.tokens.size());
  for (const auto& t : lexed.tokens) {
    if (t.kind != Kind::kUnknown) tokens.push_back(t);
  }

  Cursor c(std::move(tokens), result.diagnostics);
  Point point;

  // serve: a leading direction digit
  if (c.at(Kind::kDigit)) {
    const Token& t = c.advance();
    point.serve.direction = t.value;
    point.serve.offset = t.offset;
  } else {
    c.error(c.peek().offset, "expected a serve direction (4, 5 or 6)");
  }

  // rally: zero or more shots
  while (c.at(Kind::kShotType)) {
    point.rally.push_back(parse_shot(c));
  }

  // ending
  point.outcome = parse_ending(c);

  // anything left over
  if (!c.done()) {
    const Token& t = c.peek();
    c.error(t.offset, std::string("unexpected ") + tcl::lexer::kind_name(t.kind) +
                          " after the point ended");
  }

  result.point = std::move(point);
  return result;
}

}  // namespace tcl::parser
