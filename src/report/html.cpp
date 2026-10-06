#include "report/html.hpp"

namespace tcl::report {

std::string html_escape(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += c;
    }
  }
  return out;
}

std::string html_page(std::string_view title, std::string_view body) {
  std::string s;
  s += "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  s += "<title>" + html_escape(title) + "</title>\n";
  s += "<style>\n"
       "body { margin: 0; font: 14px/1.4 ui-monospace, Menlo, Consolas, monospace;\n"
       "       background: #161a20; color: #e6e9ef; }\n"
       "main { max-width: 980px; margin: 0 auto; padding: 24px; }\n"
       "h1 { font-size: 20px; margin: 0 0 4px; }\n"
       "h2 { font-size: 15px; margin: 28px 0 8px; color: #9aa4b2; }\n"
       "pre { background: #1d222a; padding: 12px; border-radius: 6px; overflow-x: auto; }\n"
       "</style>\n</head>\n<body>\n<main>\n";
  s += body;
  s += "\n</main>\n</body>\n</html>\n";
  return s;
}

}  // namespace tcl::report
