#include "report/report.hpp"

#include "analytics/stats.hpp"
#include "ir/shot_table.hpp"
#include "report/html.hpp"

namespace tcl::report {

namespace {

std::string summary_section(const tcl::analytics::MatchStats& st) {
  return "<h2>summary</h2>\n<pre>" + html_escape(tcl::analytics::format_report(st)) +
         "</pre>\n";
}

}  // namespace

std::string render_match_report(const std::vector<tcl::match::PointRow>& rows) {
  const auto flat = tcl::ir::flatten(rows);
  const auto stats = tcl::analytics::compute_stats(flat.points);

  const std::string title = stats.names[0] + " vs " + stats.names[1];
  std::string body;
  body += "<h1>" + html_escape(title) + "</h1>\n";
  body += "<div>" + html_escape(stats.match_id) + "</div>\n";
  body += summary_section(stats);

  return html_page(title, body);
}

}  // namespace tcl::report
