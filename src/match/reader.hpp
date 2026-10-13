#ifndef TCL_MATCH_READER_HPP
#define TCL_MATCH_READER_HPP

#include <istream>
#include <string>
#include <vector>

namespace tcl::match {

// One row of a Match Charting Project "-points" csv.
struct PointRow {
  std::string match_id;
  int pt = 0;
  int set1 = 0, set2 = 0;
  int gm1 = 0, gm2 = 0;
  std::string pts;    // running game score, e.g. "0-15"
  int gm_num = 0;
  bool tb_set = false;
  int server = 0;     // 1 or 2
  std::string first;  // "1st" column - the point, or just a fault if it faulted
  std::string second; // "2nd" column - only set if the first serve faulted
  std::string notes;
  int pt_winner = 0;  // 1 or 2
  int line_no = 0;    // 1-based source line, for error messages
};

struct ReadResult {
  std::vector<PointRow> rows; // in point order within each match, see below
  std::vector<std::string> errors; // a bad row is noted, not fatal
  int matches_reordered = 0; // how many matches had rows out of order in the file
  int duplicate_rows_dropped = 0; // exact repeats of the line right before them
  // match_ids where the same Pt number shows up more than once and the rows
  // aren't exact duplicates of each other - seen in the wild as a match
  // charted twice in full under one match_id, by two different people or in
  // two sessions. nothing here tries to untangle that, it's just flagged.
  std::vector<std::string> matches_with_repeated_points;
};

// Reads a Match Charting Project "-points" csv. Expected header:
//   match_id,Pt,Set1,Set2,Gm1,Gm2,Pts,Gm#,TbSet,Svr,1st,2nd,Notes,PtWinner
// Columns are looked up by name, so extra, missing, or reordered columns are
// fine - only "1st" is required. Rows with a bad number just keep the default
// and log an error instead of getting dropped.
//
// Rows come back in point order within each match (matches stay in the order
// they first show up). The real files aren't always in order - some matches
// have a chunk of later points sitting before the start - so replaying the
// rows as they come would score nonsense. line_no still points at the row's
// real line in the file.
//
// A line that's an exact byte-for-byte repeat of the one right before it is
// dropped (duplicate_rows_dropped counts them) - seen in the wild as the same
// point charted twice in a row, which would otherwise desync the replay by a
// point for the rest of the match.
ReadResult read_points_csv(std::istream& in);

// Whether any Pt number shows up more than once in `rows`. Meant for rows
// already narrowed to one match (read_points_csv's matches_with_repeated_points
// does the equivalent check across a whole file, grouped by match_id).
bool has_repeated_points(const std::vector<PointRow>& rows);

}  // namespace tcl::match

#endif  // TCL_MATCH_READER_HPP
