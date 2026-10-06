#include "report/html.hpp"

#include <catch2/catch_test_macros.hpp>

using tcl::report::html_escape;
using tcl::report::html_page;

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
