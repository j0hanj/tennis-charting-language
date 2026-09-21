#include "sema/outcomes.hpp"

#include <string>

namespace tcl::sema {

std::vector<ScoreIssue> check_outcomes(const std::vector<tcl::ir::PointSummary>& points) {
  std::vector<ScoreIssue> issues;
  for (const auto& p : points) {
    if (p.implied_winner == 0 || p.winner == 0) continue;
    if (p.implied_winner == p.winner) continue;
    issues.push_back({p.match_id, p.pt, p.line_no, std::to_string(p.implied_winner),
                      std::to_string(p.winner),
                      "shot string and PtWinner disagree on who won the point"});
  }
  return issues;
}

}  // namespace tcl::sema
