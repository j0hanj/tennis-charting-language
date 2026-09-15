#ifndef TCL_VIZ_COURT_HPP
#define TCL_VIZ_COURT_HPP

#include <string>
#include <string_view>
#include <vector>

#include "ast/ast.hpp"

namespace tcl::viz {

// A schematic SVG of one point: the court, plus the ball path implied by the
// shots. Positions are approximate - the notation only gives rough directions -
// so this is for a feel of the point, not a real plot.
std::string render_svg(const tcl::ast::Point& point, std::string_view source);

// A schematic SVG of a whole match (or however many points you pass in): one
// court, every shot's bounce plotted as a translucent dot, winners gold and
// errors red where the point actually ended. Same "not real tracking" caveat
// as render_svg, just aggregated.
std::string render_match_svg(const std::vector<tcl::ast::Point>& points, std::string_view title);

}  // namespace tcl::viz

#endif  // TCL_VIZ_COURT_HPP
