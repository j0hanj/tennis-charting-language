#ifndef TCL_REPORT_REPORT_HPP
#define TCL_REPORT_REPORT_HPP

#include <string>
#include <vector>

#include "match/reader.hpp"

namespace tcl::report {

// One match as a standalone HTML page. `rows` are that match's csv rows, in
// point order (as read_points_csv returns them).
std::string render_match_report(const std::vector<tcl::match::PointRow>& rows);

}  // namespace tcl::report

#endif  // TCL_REPORT_REPORT_HPP
