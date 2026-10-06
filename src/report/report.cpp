#include "report/report.hpp"

#include "analytics/stats.hpp"
#include "ir/shot_table.hpp"
#include "parser/parser.hpp"
#include "report/html.hpp"
#include "viz/court.hpp"

namespace tcl::report {

namespace {

// every shot of the match on one court. the svg is inline so the page is
// a single file with nothing else to load
std::string shot_chart_section(const std::vector<tcl::match::PointRow>& rows) {
  std::vector<tcl::ast::Point> points;
  for (const auto& row : rows) {
    const std::string& src = !row.second.empty() ? row.second : row.first;
    if (src.empty()) continue;
    if (auto pr = tcl::parser::parse(src); pr.point) points.push_back(std::move(*pr.point));
  }
  return "<h2>shot chart</h2>\n" + tcl::viz::render_match_svg(points, "every shot") + "\n";
}

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
  body += shot_chart_section(rows);

  return html_page(title, body);
}

}  // namespace tcl::report
