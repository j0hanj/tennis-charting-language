#ifndef TCL_LEXER_RENDER_HPP
#define TCL_LEXER_RENDER_HPP

#include <string>
#include <string_view>
#include <vector>

#include "lexer/token.hpp"

namespace tcl::lexer {

// Compiler-style view of one diagnostic: the source string, then a caret line
// under the offending span. e.g.
//
//   4fQf*
//     ^ don't recognize 'Q'
//
// The offset is clamped to the string so an "expected X" past the end still
// points somewhere sensible.
std::string render_diagnostic(std::string_view src, const Diagnostic& d);

// Same, for a whole batch - source printed once, a caret line per diagnostic.
std::string render_diagnostics(std::string_view src, const std::vector<Diagnostic>& ds);

}  // namespace tcl::lexer

#endif  // TCL_LEXER_RENDER_HPP
