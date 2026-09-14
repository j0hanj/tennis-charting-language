#include "sema/reconcile.hpp"

#include <cctype>
#include <utility>

#include "scoring/score.hpp"

namespace tcl::sema {

namespace {

bool ci_equal(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) return false;
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

}  // namespace

ReconcileResult reconcile_score(const std::vector<tcl::match::PointRow>& rows) {
  using tcl::scoring::MatchFormat;
  using tcl::scoring::Player;
  using tcl::scoring::Score;

  const auto fmt = MatchFormat::best_of_three_with_tiebreak();

  ReconcileResult out;
  std::string current_match;
  Score score;
  bool skipping = false;     // gave up on the current match_id, wait for the next one
  bool seeded_server = false; // whether we've taken who-serves-first from the file yet

  for (const auto& row : rows) {
    if (row.match_id != current_match) {
      current_match = row.match_id;
      score = Score{};
      skipping = false;
      seeded_server = false;
    }
    if (skipping) continue;

    if (score.finished) {
      out.issues.push_back({row.match_id, row.pt, row.line_no, "", "",
                            "match already finished by best-of-3 rules - "
                            "maybe it's best of 5?"});
      skipping = true;
      continue;
    }

    if (row.server != 1 && row.server != 2) {
      out.issues.push_back(
          {row.match_id, row.pt, row.line_no, "", "", "no server on this row, can't check it"});
      skipping = true;
      continue;
    }

    // nothing in the row data says who serves game one - take it from the
    // first row of the match and check every point against that from here on
    if (!seeded_server) {
      score.server = (row.server == 1) ? Player::kOne : Player::kTwo;
      seeded_server = true;
    }

    // who our replay thinks is serving this point - current_server() already
    // knows about the 1-then-2-at-a-time rotation inside a tiebreak
    {
      const int expected_server =
          tcl::scoring::current_server(score, fmt) == Player::kOne ? 1 : 2;
      if (row.server != expected_server) {
        out.issues.push_back({row.match_id, row.pt, row.line_no,
                              std::to_string(expected_server), std::to_string(row.server),
                              "server doesn't match"});
      }
    }

    if (!row.pts.empty()) {
      // Pts is recorded server-first; swap our internal P1/P2 order to match
      // whoever is actually serving this point before comparing.
      Score perspective = score;
      if (row.server == 2) std::swap(perspective.points[0], perspective.points[1]);
      const std::string expected = tcl::scoring::game_score(perspective, fmt);
      ++out.points_checked;
      if (!ci_equal(expected, row.pts)) {
        out.issues.push_back({row.match_id, row.pt, row.line_no, expected, row.pts,
                              "score doesn't match"});
      }
    }

    if (row.pt_winner != 1 && row.pt_winner != 2) {
      out.issues.push_back({row.match_id, row.pt, row.line_no, "", "",
                            "no PtWinner on this row, stopping here"});
      skipping = true;
      continue;
    }
    score = tcl::scoring::step(score, row.pt_winner == 1 ? Player::kOne : Player::kTwo, fmt);
  }

  return out;
}

}  // namespace tcl::sema
