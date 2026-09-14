#ifndef TCL_SEMA_RECONCILE_HPP
#define TCL_SEMA_RECONCILE_HPP

#include <string>
#include <vector>

#include "match/reader.hpp"

namespace tcl::sema {

struct ScoreIssue {
  std::string match_id;
  int pt = 0;
  int line_no = 0;
  std::string expected; // what we computed, server-first
  std::string got;      // what the row's Pts column said
  std::string message;
};

struct ReconcileResult {
  std::vector<ScoreIssue> issues;
  int points_checked = 0;
};

// Replays every row's PtWinner through the scoring engine and checks the
// result against the row's own Pts column, which records the game score
// server-first, as it stood *before* that row's point was played. Resets to
// 0-0 whenever match_id changes, so a file holding several matches works in
// one pass.
//
// Assumes best-of-three with a standard tiebreak. If a match turns out to
// need different rules (best of five, a different final-set format) the
// replay will look "finished" earlier than the file's remaining rows -
// reconciliation stops there with one issue instead of flooding every row
// after it with nonsense.
ReconcileResult reconcile_score(const std::vector<tcl::match::PointRow>& rows);

}  // namespace tcl::sema

#endif  // TCL_SEMA_RECONCILE_HPP
