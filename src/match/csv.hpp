#ifndef TCL_MATCH_CSV_HPP
#define TCL_MATCH_CSV_HPP

#include <string>
#include <string_view>
#include <vector>

namespace tcl::match {

// Splits one csv line into fields. Handles "quoted, with a comma inside" and
// "" as an escaped quote. Doesn't handle a field with an embedded newline -
// none of the columns we care about ever have one.
std::vector<std::string> split_csv_line(std::string_view line);

}  // namespace tcl::match

#endif  // TCL_MATCH_CSV_HPP
