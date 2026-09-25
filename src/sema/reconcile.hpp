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

// Replays every row's PtWinner through the scoring engine and checks it
// against two things the row already claims: the Svr column (who's serving -
// this also validates the tiebreak serve rotation against real data), and the
// Pts column, which records the game score server-first, as it stood
// *before* that row's point was played. Resets to 0-0 whenever match_id
// changes, so a file holding several matches works in one pass. Who serves
// game one is taken from the first row of each match (nothing else says it),
// then checked from there on.
//
// The file doesn't say the match format, so each match is tried against a
// short list of formats - best-of-three, best-of-five, NextGen Finals (short
// sets, no-ad), and the best-of-3/5 variants where the last set is decided by
// a first-to-10 breaker instead of being played out (wimbledon, the AO) -
// keeping whichever leaves the fewest issues. Anything stranger still shows
// up as issues - reconciliation stops at the first row it can't make sense of
// instead of flooding every row after it with nonsense.
ReconcileResult reconcile_score(const std::vector<tcl::match::PointRow>& rows);

}  // namespace tcl::sema

#endif  // TCL_SEMA_RECONCILE_HPP
