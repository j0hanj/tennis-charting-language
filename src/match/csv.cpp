#include "match/csv.hpp"

namespace tcl::match {

std::vector<std::string> split_csv_line(std::string_view line) {
  std::vector<std::string> fields;
  std::string cur;
  bool in_quotes = false;

  for (std::size_t i = 0; i < line.size(); ++i) {
    const char c = line[i];
    if (in_quotes) {
      if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
        cur += '"';
        ++i;
      } else if (c == '"') {
        in_quotes = false;
      } else {
        cur += c;
      }
    } else if (c == '"') {
      in_quotes = true;
    } else if (c == ',') {
      fields.push_back(cur);
      cur.clear();
    } else {
      cur += c;
    }
  }
  fields.push_back(cur);
  return fields;
}

}  // namespace tcl::match
