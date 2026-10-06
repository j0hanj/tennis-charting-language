#ifndef TCL_REPORT_HTML_HPP
#define TCL_REPORT_HTML_HPP

#include <string>
#include <string_view>

namespace tcl::report {

// Escape text for use inside HTML element content or a quoted attribute.
std::string html_escape(std::string_view text);

// Wrap a body in a complete, standalone HTML document with inline CSS.
std::string html_page(std::string_view title, std::string_view body);

}  // namespace tcl::report

#endif  // TCL_REPORT_HTML_HPP
