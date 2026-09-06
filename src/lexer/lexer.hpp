#ifndef TCL_LEXER_LEXER_HPP
#define TCL_LEXER_LEXER_HPP

#include <string_view>
#include <vector>

#include "lexer/token.hpp"

namespace tcl::lexer {

struct Lexed {
  std::vector<Token> tokens;           // always ends with a kEnd token
  std::vector<Diagnostic> diagnostics; // unknown characters, etc.

  bool ok() const { return diagnostics.empty(); }
};

// Scan a single charting string (one point). Unknown characters become kUnknown
// tokens with a diagnostic; scanning keeps going so one bad character doesn't
// lose the rest of the string. Whitespace is skipped.
Lexed lex(std::string_view src);

}  // namespace tcl::lexer

#endif  // TCL_LEXER_LEXER_HPP
