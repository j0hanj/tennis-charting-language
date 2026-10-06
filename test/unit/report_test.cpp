#include "report/html.hpp"
#include "report/report.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using tcl::match::PointRow;
using tcl::report::html_escape;
using tcl::report::html_page;
using tcl::report::render_match_report;

namespace {

PointRow row(int pt, std::string first, std::string second, int server, int winner) {
  PointRow r;
  r.match_id = "20240101-M-Test-R1-Ann_Lee-Bo_Park";
  r.pt = pt;
  r.first = std::move(first);
  r.second = std::move(second);
  r.server = server;
  r.pt_winner = winner;
  return r;
}

}  // namespace

TEST_CASE("html_escape handles the five special characters", "[report]") {
  CHECK(html_escape("a<b>&\"'") == "a&lt;b&gt;&amp;&quot;&#39;");
  CHECK(html_escape("4ffbbf*") == "4ffbbf*");
}

TEST_CASE("html_page wraps the body in a full document", "[report]") {
  const std::string page = html_page("a & b", "<p>hi</p>");
  CHECK(page.rfind("<!doctype html>", 0) == 0);
  CHECK(page.find("<title>a &amp; b</title>") != std::string::npos);
  CHECK(page.find("<p>hi</p>") != std::string::npos);
  CHECK(page.find("</html>") != std::string::npos);
}

TEST_CASE("the report names the players and has a summary", "[report]") {
  const std::vector<PointRow> rows{row(1, "4*", "", 1, 1), row(2, "6f2b*", "", 2, 2)};
  const std::string html = render_match_report(rows);
  CHECK(html.find("<title>Ann Lee vs Bo Park</title>") != std::string::npos);
  CHECK(html.find("<h2>summary</h2>") != std::string::npos);
  CHECK(html.find("points won") != std::string::npos);
}
