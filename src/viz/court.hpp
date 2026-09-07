#ifndef TCL_VIZ_COURT_HPP
#define TCL_VIZ_COURT_HPP

#include <string>
#include <string_view>

#include "ast/ast.hpp"

namespace tcl::viz {

// A schematic SVG of one point: the court, plus the ball path implied by the
// shots. Positions are approximate - the notation only gives rough directions -
// so this is for a feel of the point, not a real plot.
std::string render_svg(const tcl::ast::Point& point, std::string_view source);

}  // namespace tcl::viz

#endif  // TCL_VIZ_COURT_HPP
