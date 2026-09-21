#include "sema/reconcile.hpp"

#include <cctype>
#include <utility>

#include "scoring/score.hpp"

namespace tcl::sema {

namespace {

using tcl::scoring::MatchFormat;
using tcl::scoring::Player;
using tcl::scoring::Score;

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

// Replay rows[begin, end) - one match - under one set of rules and report
// everything that doesn't line up. Stops at the first row it can't make sense
// of (no server, no winner, match already over) rather than flooding the rest
// with knock-on errors.
ReconcileResult replay(const std::vector<tcl::match::PointRow>& rows, std::size_t begin,
                       std::size_t end, const MatchFormat& fmt) {
  ReconcileResult out;
  Score score;
  bool seeded_server = false;

  for (std::size_t i = begin; i < end; ++i) {
    const auto& row = rows[i];

    if (score.finished) {
      out.issues.push_back({row.match_id, row.pt, row.line_no, "", "",
                            "match already finished by best-of-3 rules - "
                            "maybe it's best of 5?"});
      return out;
    }

    if (row.server != 1 && row.server != 2) {
      out.issues.push_back(
          {row.match_id, row.pt, row.line_no, "", "", "no server on this row, can't check it"});
      return out;
    }

    // nothing in the row data says who serves game one - take it from the
    // first row of the match and check every point against that from here on
    if (!seeded_server) {
      score.server = (row.server == 1) ? Player::kOne : Player::kTwo;
      seeded_server = true;
    }

    // who our replay thinks is serving this point - current_server() already
    // knows about the 1-then-2-at-a-time rotation inside a tiebreak
    const int expected_server = tcl::scoring::current_server(score, fmt) == Player::kOne ? 1 : 2;
    if (row.server != expected_server) {
      out.issues.push_back({row.match_id, row.pt, row.line_no, std::to_string(expected_server),
                            std::to_string(row.server), "server doesn't match"});
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
      return out;
    }
    score = tcl::scoring::step(score, row.pt_winner == 1 ? Player::kOne : Player::kTwo, fmt);
  }

  return out;
}

}  // namespace

ReconcileResult reconcile_score(const std::vector<tcl::match::PointRow>& rows) {
  ReconcileResult out;

  std::size_t begin = 0;
  while (begin < rows.size()) {
    std::size_t end = begin;
    while (end < rows.size() && rows[end].match_id == rows[begin].match_id) ++end;

    // nothing in the file says how many sets the match was, so try the
    // common case first and only fall back to best of five if that doesn't fit
    ReconcileResult best = replay(rows, begin, end, MatchFormat::best_of_three_with_tiebreak());
    if (!best.issues.empty()) {
      ReconcileResult five = replay(rows, begin, end, MatchFormat::best_of_five_with_tiebreak());
      if (five.issues.size() < best.issues.size()) best = std::move(five);
    }

    out.points_checked += best.points_checked;
    out.issues.insert(out.issues.end(), best.issues.begin(), best.issues.end());
    begin = end;
  }

  return out;
}

}  // namespace tcl::sema
