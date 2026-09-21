#include "ir/shot_table.hpp"

#include "parser/parser.hpp"

namespace tcl::ir {

namespace {

using tcl::ast::Ender;

int other_player(int p) { return p == 1 ? 2 : (p == 2 ? 1 : 0); }

// shot 1 is the serve, then it alternates: returner on the even ones
int hitter_of(int server, int shot_no) {
  if (server != 1 && server != 2) return 0;
  return shot_no % 2 == 1 ? server : other_player(server);
}

// a serve that missed: just a direction and a fault code, nothing else
bool is_bare_fault(const tcl::ast::Point& p) { return p.fault.has_value(); }

}  // namespace

Flattened flatten(const std::vector<tcl::match::PointRow>& rows) {
  Flattened out;

  for (const auto& row : rows) {
    const bool has_second = !row.second.empty();
    const std::string& src = has_second ? row.second : row.first;
    if (src.empty()) continue;

    const auto parsed = tcl::parser::parse(src);
    if (!parsed.point) continue;
    const tcl::ast::Point& p = *parsed.point;

    PointSummary ps;
    ps.match_id = row.match_id;
    ps.pt = row.pt;
    ps.line_no = row.line_no;
    ps.server = row.server;
    ps.winner = row.pt_winner;
    ps.second_serve = has_second;
    ps.first_serve_fault = has_second; // a 2nd column only exists if the 1st missed
    ps.serve_dir = p.serve.direction.value_or(0);

    const int server = row.server;
    const int returner = other_player(server);

    if (is_bare_fault(p)) {
      ps.double_fault = has_second;
      ps.rally_len = 1;
      ps.last_hitter = server;
      if (ps.double_fault) ps.implied_winner = returner;
    } else {
      ps.rally_len = 1 + static_cast<int>(p.rally.size());
      ps.last_hitter = hitter_of(server, ps.rally_len);
      // "4#" / "4@" - a serve with an error marker and no return charted. the
      // error is the returner's (they couldn't get it back), not the server's
      if (p.rally.empty() && p.outcome && p.outcome->ender != Ender::kWinner) {
        ps.last_hitter = returner;
      }
      if (p.outcome) {
        ps.ender = p.outcome->ender;
        ps.error_loc = p.outcome->where;
        ps.ace = p.rally.empty() && p.outcome->ender == Ender::kWinner;
        // a winner means the last hitter took it, an error means they lost it
        if (ps.last_hitter != 0) {
          ps.implied_winner = p.outcome->ender == Ender::kWinner ? ps.last_hitter
                                                                 : other_player(ps.last_hitter);
        }
      }
    }
    out.points.push_back(ps);

    // the serve row
    ShotRow serve;
    serve.match_id = row.match_id;
    serve.pt = row.pt;
    serve.shot_no = 1;
    serve.hitter = server;
    serve.is_serve = true;
    serve.direction = ps.serve_dir;
    serve.second_serve = has_second;
    serve.point_winner = row.pt_winner;
    serve.rally_len = ps.rally_len;
    serve.is_last = p.rally.empty();
    if (serve.is_last) {
      serve.ender = ps.ender;
      serve.error_loc = ps.error_loc;
    }
    out.shots.push_back(serve);

    // then each rally shot
    for (std::size_t k = 0; k < p.rally.size(); ++k) {
      const auto& sh = p.rally[k];
      ShotRow r;
      r.match_id = row.match_id;
      r.pt = row.pt;
      r.shot_no = static_cast<int>(k) + 2;
      r.hitter = hitter_of(server, r.shot_no);
      r.type = sh.type;
      r.direction = sh.direction.value_or(0);
      r.depth = sh.depth.value_or(0);
      r.position = sh.position.value_or(0);
      r.second_serve = has_second;
      r.point_winner = row.pt_winner;
      r.rally_len = ps.rally_len;
      r.is_last = (k + 1 == p.rally.size());
      if (r.is_last) {
        r.ender = ps.ender;
        r.error_loc = ps.error_loc;
      }
      out.shots.push_back(r);
    }
  }

  return out;
}

std::string shots_csv_header() {
  return "match_id,pt,shot_no,hitter,type,direction,depth,position,is_last,ender,"
         "error_loc,second_serve,point_winner,rally_len";
}

std::string to_csv_line(const ShotRow& r) {
  auto opt_num = [](int v) { return v == 0 ? std::string() : std::to_string(v); };
  auto opt_chr = [](char c) { return c == 0 ? std::string() : std::string(1, c); };

  std::string s = r.match_id;
  s += ',' + std::to_string(r.pt);
  s += ',' + std::to_string(r.shot_no);
  s += ',' + opt_num(r.hitter);
  s += ',' + (r.is_serve ? std::string("serve") : opt_chr(r.type));
  s += ',' + opt_num(r.direction);
  s += ',' + opt_num(r.depth);
  s += ',' + opt_chr(r.position);
  s += ',' + std::string(r.is_last ? "1" : "0");
  s += ',' + (r.ender ? std::string(tcl::ast::ender_name(*r.ender)) : std::string());
  s += ',' + (r.error_loc ? std::string(tcl::ast::error_loc_name(*r.error_loc)) : std::string());
  s += ',' + std::string(r.second_serve ? "1" : "0");
  s += ',' + opt_num(r.point_winner);
  s += ',' + std::to_string(r.rally_len);
  return s;
}

}  // namespace tcl::ir
