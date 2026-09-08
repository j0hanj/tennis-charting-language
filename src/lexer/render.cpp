#include "lexer/render.hpp"

#include <algorithm>

namespace tcl::lexer {

namespace {

constexpr std::string_view kIndent = "  ";

// A caret line: indent, spaces up to `offset`, then `length` carets, then the
// message. No trailing newline.
std::string caret_line(std::string_view src, const Diagnostic& d) {
  const std::size_t at = std::min(d.offset, src.size());
  const std::size_t len = std::max<std::size_t>(1, d.length);

  std::string line(kIndent);
  line.append(at, ' ');
  line.append(len, '^');
  if (!d.message.empty()) {
    line += ' ';
    line += d.message;
  }
  return line;
}

}  // namespace

std::string render_diagnostic(std::string_view src, const Diagnostic& d) {
  std::string out(kIndent);
  out += src;
  out += '\n';
  out += caret_line(src, d);
  return out;
}

std::string render_diagnostics(std::string_view src, const std::vector<Diagnostic>& ds) {
  std::string out(kIndent);
  out += src;
  for (const auto& d : ds) {
    out += '\n';
    out += caret_line(src, d);
  }
  return out;
}

}  // namespace tcl::lexer
