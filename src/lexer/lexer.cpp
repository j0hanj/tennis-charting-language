#include "lexer/lexer.hpp"

#include <cctype>
#include <string>

namespace tcl::lexer {

namespace {

constexpr std::string_view kShotTypes = "fbrsvzopluyhi";
constexpr std::string_view kErrorLocs = "nwdx";

bool contains(std::string_view set, char c) {
  return set.find(c) != std::string_view::npos;
}

}  // namespace

const char* kind_name(Kind k) {
  switch (k) {
    case Kind::kShotType:  return "shot";
    case Kind::kDigit:     return "digit";
    case Kind::kPosition:  return "position";
    case Kind::kEndMarker: return "end-marker";
    case Kind::kErrorLoc:  return "error-loc";
    case Kind::kUnknown:   return "unknown";
    case Kind::kEnd:       return "end";
  }
  return "?";
}

Lexed lex(std::string_view src) {
  Lexed out;

  for (std::size_t i = 0; i < src.size(); ++i) {
    const char c = src[i];
    if (std::isspace(static_cast<unsigned char>(c))) {
      continue;
    }

    Token t;
    t.offset = i;
    t.text = src.substr(i, 1);

    if (c >= '1' && c <= '9') {
      t.kind = Kind::kDigit;
      t.value = c - '0';
    } else if (contains(kShotTypes, c)) {
      t.kind = Kind::kShotType;
    } else if (c == '+' || c == '-' || c == '=') {
      t.kind = Kind::kPosition;
    } else if (c == '*' || c == '@' || c == '#') {
      t.kind = Kind::kEndMarker;
    } else if (contains(kErrorLocs, c)) {
      t.kind = Kind::kErrorLoc;
    } else {
      t.kind = Kind::kUnknown;
      out.diagnostics.push_back(
          {i, 1, std::string("don't recognize '") + c + "'"});
    }

    out.tokens.push_back(t);
  }

  out.tokens.push_back(Token{Kind::kEnd, {}, src.size(), 0});
  return out;
}

}  // namespace tcl::lexer
