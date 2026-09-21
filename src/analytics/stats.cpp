#include "analytics/stats.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>

namespace tcl::analytics {

namespace {

using tcl::ast::Ender;

std::string pct(int num, int den) {
  if (den == 0) return "-";
  char buf[16];
  std::snprintf(buf, sizeof buf, "%.0f%%", 100.0 * num / den);
  return buf;
}

std::string pad(const std::string& s, std::size_t width) {
  return s.size() >= width ? s : s + std::string(width - s.size(), ' ');
}

std::string rpad(const std::string& s, std::size_t width) {
  return s.size() >= width ? s : std::string(width - s.size(), ' ') + s;
}

std::size_t bucket_for(int len) {
  if (len <= 3) return 0;
  if (len <= 6) return 1;
  if (len <= 9) return 2;
  return 3;
}

}  // namespace

std::array<std::string, 2> names_from_match_id(const std::string& match_id) {
  // date-M-Tournament-Round-Player_One-Player_Two
  std::vector<std::string> parts;
  std::string cur;
  for (const char c : match_id) {
    if (c == '-') {
      parts.push_back(cur);
      cur.clear();
    } else {
      cur += c;
    }
  }
  parts.push_back(cur);

  if (parts.size() != 6 || parts[4].empty() || parts[5].empty()) return {"P1", "P2"};

  auto pretty = [](std::string s) {
    std::replace(s.begin(), s.end(), '_', ' ');
    return s;
  };
  return {pretty(parts[4]), pretty(parts[5])};
}

MatchStats compute_stats(const std::vector<tcl::ir::PointSummary>& points) {
  MatchStats st;
  st.by_length = {LengthBucket{"1-3"}, LengthBucket{"4-6"}, LengthBucket{"7-9"},
                  LengthBucket{"10+"}};
  if (points.empty()) return st;

  st.match_id = points.front().match_id;
  st.names = names_from_match_id(st.match_id);

  long long len_sum = 0;
  int len_count = 0;

  for (const auto& p : points) {
    ++st.points;

    const int s = p.server - 1; // index into player[], -1 if the server's unknown
    const bool server_ok = (s == 0 || s == 1);

    if (p.winner == 1 || p.winner == 2) ++st.player[static_cast<std::size_t>(p.winner - 1)].points_won;
    if (p.implied_winner != 0 && p.winner != 0 && p.implied_winner != p.winner) {
      ++st.disagreements;
    }

    if (server_ok) {
      auto& sp = st.player[static_cast<std::size_t>(s)];
      ++sp.service_points;
      if (p.winner == p.server) ++sp.service_points_won;
      if (!p.first_serve_fault) ++sp.first_serves_in;
      if (p.double_fault) ++sp.double_faults;
      if (p.ace) ++sp.aces;
    }

    if (p.double_fault) continue; // no rally to speak of

    ++st.rally_hist[p.rally_len];
    len_sum += p.rally_len;
    ++len_count;
    st.longest = std::max(st.longest, p.rally_len);

    auto& bucket = st.by_length[bucket_for(p.rally_len)];
    ++bucket.points;

    if (p.ender && (p.last_hitter == 1 || p.last_hitter == 2)) {
      auto& hp = st.player[static_cast<std::size_t>(p.last_hitter - 1)];
      switch (*p.ender) {
        case Ender::kWinner:
          if (!p.ace) ++hp.winners;
          ++bucket.winners;
          break;
        case Ender::kUnforcedError:
          ++hp.unforced;
          ++bucket.unforced;
          break;
        case Ender::kForcedError:
          ++hp.forced;
          ++bucket.forced;
          break;
      }
    }

    if (p.serve_dir >= 4 && p.serve_dir <= 6) {
      auto& sd = st.serve_dir[static_cast<std::size_t>(p.serve_dir - 4)];
      ++sd.points;
      if (p.winner == p.server) ++sd.won;
      if (p.ace) ++sd.aces;
    }
  }

  if (len_count > 0) st.avg_rally = static_cast<double>(len_sum) / len_count;
  return st;
}

std::vector<std::vector<tcl::ir::PointSummary>> split_by_match(
    const std::vector<tcl::ir::PointSummary>& points) {
  std::vector<std::vector<tcl::ir::PointSummary>> out;
  for (const auto& p : points) {
    if (out.empty() || out.back().front().match_id != p.match_id) out.emplace_back();
    out.back().push_back(p);
  }
  return out;
}

std::string format_report(const MatchStats& s) {
  std::ostringstream o;
  const std::size_t col = std::max<std::size_t>({s.names[0].size(), s.names[1].size(), 10}) + 2;

  o << s.names[0] << " vs " << s.names[1] << "  (" << s.points << " points)\n\n";

  auto row = [&](const std::string& label, const std::string& a, const std::string& b) {
    o << "  " << pad(label, 22) << pad(a, col) << b << '\n';
  };
  const auto& a = s.player[0];
  const auto& b = s.player[1];

  row("", s.names[0], s.names[1]);
  row("points won", std::to_string(a.points_won), std::to_string(b.points_won));
  row("first serve in", pct(a.first_serves_in, a.service_points),
      pct(b.first_serves_in, b.service_points));
  row("won on serve", pct(a.service_points_won, a.service_points),
      pct(b.service_points_won, b.service_points));
  row("aces", std::to_string(a.aces), std::to_string(b.aces));
  row("double faults", std::to_string(a.double_faults), std::to_string(b.double_faults));
  row("winners", std::to_string(a.winners), std::to_string(b.winners));
  row("unforced errors", std::to_string(a.unforced), std::to_string(b.unforced));
  row("forced errors", std::to_string(a.forced), std::to_string(b.forced));

  // rally length histogram, serve counts as shot 1. everything past 10 lumped
  o << "\nrally length (shots per point, serve = 1)\n";
  std::array<int, 11> hist{}; // index 1..10, 0 unused, 10 means 10+
  for (const auto& [len, n] : s.rally_hist) hist[static_cast<std::size_t>(std::min(len, 10))] += n;
  const int top = *std::max_element(hist.begin(), hist.end());
  for (int len = 1; len <= 10; ++len) {
    const int n = hist[static_cast<std::size_t>(len)];
    const int bar = top == 0 ? 0 : (n * 30 + top - 1) / top;
    o << "  " << rpad(len == 10 ? "10+" : std::to_string(len), 3) << "  " << pad(std::string(static_cast<std::size_t>(bar), '#'), 31)
      << n << '\n';
  }
  char buf[64];
  std::snprintf(buf, sizeof buf, "  avg %.1f shots, longest %d\n", s.avg_rally, s.longest);
  o << buf;

  o << "\nhow points end, by rally length\n";
  o << "  " << pad("shots", 8) << rpad("points", 7) << rpad("winners", 9) << rpad("unforced", 10)
    << rpad("forced", 8) << rpad("ue rate", 9) << '\n';
  for (const auto& bk : s.by_length) {
    o << "  " << pad(bk.label, 8) << rpad(std::to_string(bk.points), 7)
      << rpad(std::to_string(bk.winners), 9) << rpad(std::to_string(bk.unforced), 10)
      << rpad(std::to_string(bk.forced), 8) << rpad(pct(bk.unforced, bk.points), 9) << '\n';
  }

  o << "\nserve direction (server's win rate)\n";
  const char* names[3] = {"wide (4)", "body (5)", "T (6)"};
  for (std::size_t i = 0; i < 3; ++i) {
    const auto& d = s.serve_dir[i];
    o << "  " << pad(names[i], 10) << rpad(std::to_string(d.points), 4) << " pts  "
      << rpad(pct(d.won, d.points), 4) << " won  " << d.aces << " aces\n";
  }

  if (s.disagreements > 0) {
    o << "\n" << s.disagreements
      << " point(s) where the shot string and PtWinner disagree - see `tcl lint`\n";
  }
  return o.str();
}

}  // namespace tcl::analytics
