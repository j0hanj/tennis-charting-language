// The flat form of a charted match: one summary per point, one row per shot.
//
// stats and the csv export read this, never the parse tree. that's the whole
// point of flattening - everything downstream can just loop over rows.

#ifndef TCL_IR_SHOT_TABLE_HPP
#define TCL_IR_SHOT_TABLE_HPP

#include <optional>
#include <string>
#include <vector>

#include "ast/ast.hpp"
#include "match/reader.hpp"

namespace tcl::ir {

struct ShotRow {
  std::string match_id;
  int pt = 0;
  int shot_no = 0;          // 1 = the serve, 2 = the return, ...
  int hitter = 0;           // 1 or 2 (the file's player numbering), 0 if unknown
  bool is_serve = false;
  char type = 0;            // f b r s ..., 0 on a serve row
  int direction = 0;        // serve: 4/5/6. rally: 1/2/3. 0 = not given
  int depth = 0;            // 7/8/9 on a return, 0 = not given
  char position = 0;        // + - =, 0 = not given
  bool is_last = false;
  std::optional<tcl::ast::Ender> ender;         // only on the last shot. on a serve row
                                                // that means the *returner's* error
  std::optional<tcl::ast::ErrorLoc> error_loc;  // only on the last shot
  bool second_serve = false;
  int point_winner = 0;     // straight from PtWinner
  int rally_len = 0;        // shots in the point, serve included
};

struct PointSummary {
  std::string match_id;
  int pt = 0;
  int line_no = 0;
  int server = 0;
  int winner = 0;             // PtWinner, as the file says
  int implied_winner = 0;     // who the shot string says won, 0 if it can't tell
  bool second_serve = false;  // the point was played off a second serve
  bool first_serve_fault = false;
  bool double_fault = false;
  bool ace = false;
  int serve_dir = 0;
  int rally_len = 0;          // shots including the serve
  int last_hitter = 0;
  std::optional<tcl::ast::Ender> ender;
  std::optional<tcl::ast::ErrorLoc> error_loc;
};

struct Flattened {
  std::vector<PointSummary> points;
  std::vector<ShotRow> shots;
};

// Parse every row's point (the 2nd column if the first serve faulted, else the
// 1st) and flatten it. Rows with nothing to parse are skipped. Who hit each shot
// comes from the alternation: server serves, returner hits shot 2, and so on.
Flattened flatten(const std::vector<tcl::match::PointRow>& rows);

// csv export of the shot table.
std::string shots_csv_header();
std::string to_csv_line(const ShotRow& row);

}  // namespace tcl::ir

#endif  // TCL_IR_SHOT_TABLE_HPP
