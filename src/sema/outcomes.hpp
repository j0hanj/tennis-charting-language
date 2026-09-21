#ifndef TCL_SEMA_OUTCOMES_HPP
#define TCL_SEMA_OUTCOMES_HPP

#include <vector>

#include "ir/shot_table.hpp"
#include "sema/reconcile.hpp"

namespace tcl::sema {

// The shot string and the PtWinner column are two separate records of the same
// thing - if the last shot was a winner, whoever hit it should be the one who
// won the point; if it was an error they should've lost it; a double fault
// always goes to the returner. When they disagree one of them is a charting
// mistake. Points where the string can't say who won are skipped.
//
// Reuses ScoreIssue: `expected` is who the shot string implies, `got` is
// PtWinner (both as "1" / "2").
std::vector<ScoreIssue> check_outcomes(const std::vector<tcl::ir::PointSummary>& points);

}  // namespace tcl::sema

#endif  // TCL_SEMA_OUTCOMES_HPP
