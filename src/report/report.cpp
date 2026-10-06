#include "report/report.hpp"

#include "analytics/stats.hpp"
#include "ir/shot_table.hpp"
#include "parser/parser.hpp"
#include "report/html.hpp"
#include "scoring/score.hpp"
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

// one line per point: the running scoreline after it was played. replays the
// file's own PtWinner through the scoring engine (best of 3, tiebreak) - the
// same replay lint uses, just shown instead of checked
std::string timeline_section(const std::vector<tcl::match::PointRow>& rows) {
  using tcl::scoring::MatchFormat;
  using tcl::scoring::Player;
  const auto fmt = MatchFormat::best_of_three_with_tiebreak();

  std::string out = "<h2>score timeline</h2>\n<pre>";
  tcl::scoring::Score score;
  for (const auto& row : rows) {
    if (row.pt_winner != 1 && row.pt_winner != 2) continue;
    if (score.finished) break;
    score = tcl::scoring::step(score, row.pt_winner == 1 ? Player::kOne : Player::kTwo, fmt);
    out += html_escape(std::to_string(row.pt)) + "  " + html_escape(scoreline(score, fmt)) + "\n";
  }
  out += "</pre>\n";
  return out;
}

// every point as a collapsible row. the summary line is the point number,
// server and result; opening it shows that point's own court diagram
std::string points_section(const std::vector<tcl::match::PointRow>& rows) {
  std::string out = "<h2>points</h2>\n";
  for (const auto& row : rows) {
    const std::string& src = !row.second.empty() ? row.second : row.first;
    if (src.empty()) continue;
    const auto pr = tcl::parser::parse(src);
    if (!pr.point) continue;

    const std::string head = "point " + std::to_string(row.pt) + "  serve " +
                             std::to_string(row.server) + "  winner " +
                             std::to_string(row.pt_winner) + "  " + src;
    out += "<details><summary>" + html_escape(head) + "</summary>\n";
    out += tcl::viz::render_svg(*pr.point, src) + "</details>\n";
  }
  return out;
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
  body += timeline_section(rows);
  body += points_section(rows);

  return html_page(title, body);
}

}  // namespace tcl::report
