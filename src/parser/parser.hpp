#ifndef TCL_PARSER_PARSER_HPP
#define TCL_PARSER_PARSER_HPP

#include <optional>
#include <string_view>
#include <vector>

#include "ast/ast.hpp"
#include "lexer/token.hpp"

namespace tcl::parser {

struct Parsed {
  std::optional<tcl::ast::Point> point;      // a best-effort tree, even on errors
  std::vector<tcl::lexer::Diagnostic> diagnostics;

  bool ok() const { return diagnostics.empty(); }
};

// Parse one charting string (one serve + rally). Hand-written recursive descent.
// Always returns whatever tree it managed to build; problems come back as
// diagnostics pointing at the offending offset.
Parsed parse(std::string_view src);

}  // namespace tcl::parser

#endif  // TCL_PARSER_PARSER_HPP
