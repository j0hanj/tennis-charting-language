// Tokens the lexer produces from a charting string.
//
// Every token here is a single character. The digit kinds (serve direction,
// rally direction, return depth) all look the same at this level - they're just
// kDigit with a value, and the parser works out which one it is from position.

#ifndef TCL_LEXER_TOKEN_HPP
#define TCL_LEXER_TOKEN_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace tcl::lexer {

enum class Kind {
  kShotType,  // f b r s v z o p l u y h i
  kDigit,     // 1-9
  kPosition,  // + (approach)  - (at net)  = (at baseline)
  kEndMarker, // * (winner)  @ (unforced)  # (forced)
  kErrorLoc,  // n (net)  w (wide)  d (deep)  x (wide and deep)
  kUnknown,   // a character we don't know
  kEnd,       // end of input
};

const char* kind_name(Kind k);

struct Token {
  Kind kind = Kind::kEnd;
  std::string_view text;  // slice of the source (empty for kEnd)
  std::size_t offset = 0; // byte offset into the source string
  int value = 0;          // digit value, only set when kind == kDigit
};

struct Diagnostic {
  std::size_t offset = 0;
  std::size_t length = 1;
  std::string message;
};

}  // namespace tcl::lexer

#endif  // TCL_LEXER_TOKEN_HPP
