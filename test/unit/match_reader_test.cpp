#include "match/reader.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

#include "match/csv.hpp"

using tcl::match::read_points_csv;
using tcl::match::split_csv_line;

TEST_CASE("split_csv_line handles plain and quoted fields", "[match]") {
  CHECK(split_csv_line("a,b,c") == std::vector<std::string>{"a", "b", "c"});
  CHECK(split_csv_line("") == std::vector<std::string>{""});
  CHECK(split_csv_line("a,,c") == std::vector<std::string>{"a", "", "c"});
  CHECK(split_csv_line(R"("hi, there",b)") == std::vector<std::string>{"hi, there", "b"});
  CHECK(split_csv_line(R"("say ""hi""",b)") == std::vector<std::string>{"say \"hi\"", "b"});
}

TEST_CASE("reads a small points csv by column name", "[match]") {
  std::istringstream in(
      "match_id,Pt,Set1,Set2,Gm1,Gm2,Pts,Gm#,TbSet,Svr,1st,2nd,Notes,PtWinner\n"
      "m1,1,0,0,0,0,0-0,1,True,1,4ffbbf*,,,1\n"
      "m1,2,0,0,0,0,15-0,1,True,1,6n,5f18f*,,1\n");

  const auto r = read_points_csv(in);
  REQUIRE(r.errors.empty());
  REQUIRE(r.rows.size() == 2);

  CHECK(r.rows[0].match_id == "m1");
  CHECK(r.rows[0].pt == 1);
  CHECK(r.rows[0].tb_set == true);
  CHECK(r.rows[0].server == 1);
  CHECK(r.rows[0].first == "4ffbbf*");
  CHECK(r.rows[0].second.empty());
  CHECK(r.rows[0].pts == "0-0");
  CHECK(r.rows[0].pt_winner == 1);
  CHECK(r.rows[0].line_no == 2);

  CHECK(r.rows[1].first == "6n");     // a faulted first serve
  CHECK(r.rows[1].second == "5f18f*"); // the point that was actually played
}

TEST_CASE("columns can be reordered and extra ones are ignored", "[match]") {
  std::istringstream in(
      "extra,1st,PtWinner\n"
      "whatever,4*,1\n");
  const auto r = read_points_csv(in);
  REQUIRE(r.errors.empty());
  REQUIRE(r.rows.size() == 1);
  CHECK(r.rows[0].first == "4*");
  CHECK(r.rows[0].pt_winner == 1);
}

TEST_CASE("a missing 1st column is a hard error", "[match]") {
  std::istringstream in("match_id,Pt\nm1,1\n");
  const auto r = read_points_csv(in);
  REQUIRE(r.errors.size() == 1);
  CHECK(r.rows.empty());
}

TEST_CASE("a bad number is logged but doesn't drop the row", "[match]") {
  std::istringstream in("1st,Pt\n4*,not-a-number\n");
  const auto r = read_points_csv(in);
  REQUIRE(r.rows.size() == 1);
  CHECK(r.rows[0].pt == 0); // left at the default
  REQUIRE(r.errors.size() == 1);
  CHECK(r.errors[0].find("line 2") != std::string::npos);
}

TEST_CASE("blank lines are skipped", "[match]") {
  std::istringstream in("1st\n4*\n\n6b*\n");
  const auto r = read_points_csv(in);
  CHECK(r.rows.size() == 2);
}
