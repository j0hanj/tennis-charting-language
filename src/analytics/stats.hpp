#ifndef TCL_ANALYTICS_STATS_HPP
#define TCL_ANALYTICS_STATS_HPP

#include <array>
#include <map>
#include <string>
#include <vector>

#include "ir/shot_table.hpp"

namespace tcl::analytics {

struct PlayerStats {
  int service_points = 0;
  int service_points_won = 0;
  int first_serves_in = 0;
  int aces = 0;
  int double_faults = 0;
  int winners = 0;   // not counting aces
  int unforced = 0;
  int forced = 0;    // forced errors this player made
  int points_won = 0;
};

struct LengthBucket {
  std::string label;
  int points = 0;
  int winners = 0;
  int unforced = 0;
  int forced = 0;
};

struct ServeDirStats {
  int points = 0;
  int won = 0; // by the server
  int aces = 0;
};

struct MatchStats {
  std::string match_id;
  std::array<std::string, 2> names{"P1", "P2"};
  int points = 0;
  std::array<PlayerStats, 2> player;
  std::map<int, int> rally_hist; // shots per point (serve = 1) -> count, no double faults
  double avg_rally = 0.0;
  int longest = 0;
  std::array<LengthBucket, 4> by_length; // 1-3, 4-6, 7-9, 10+
  std::array<ServeDirStats, 3> serve_dir; // 4 wide, 5 body, 6 T
  int disagreements = 0; // shot string says one winner, PtWinner says the other
};

// Player names out of a match_id like
// "20260521-M-Roland_Garros-Q3-Jesper_De_Jong-Michael_Zheng". Falls back to
// P1 / P2 if it isn't shaped like that.
std::array<std::string, 2> names_from_match_id(const std::string& match_id);

// One match's worth of point summaries (all the same match_id) -> stats.
MatchStats compute_stats(const std::vector<tcl::ir::PointSummary>& points);

// Split a file's summaries into one vector per match_id, in file order.
std::vector<std::vector<tcl::ir::PointSummary>> split_by_match(
    const std::vector<tcl::ir::PointSummary>& points);

// The text report `tcl stats` prints.
std::string format_report(const MatchStats& s);

}  // namespace tcl::analytics

#endif  // TCL_ANALYTICS_STATS_HPP
