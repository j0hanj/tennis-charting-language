#include "match/reader.hpp"

#include <algorithm>
#include <numeric>
#include <optional>
#include <string>
#include <unordered_map>

#include "match/csv.hpp"

namespace tcl::match {

namespace {

std::optional<int> to_int(const std::string& s) {
  if (s.empty()) return std::nullopt;
  try {
    std::size_t consumed = 0;
    const int v = std::stoi(s, &consumed);
    if (consumed != s.size()) return std::nullopt;
    return v;
  } catch (...) {
    return std::nullopt;
  }
}

// Put each match's rows in Pt order without shuffling the matches themselves.
// A match where any row is missing its Pt is left exactly as the file had it,
// since there's nothing safe to sort by.
void sort_into_point_order(ReadResult& r) {
  std::unordered_map<std::string, std::size_t> match_index; // first appearance
  std::unordered_map<std::string, bool> sortable;
  for (const auto& row : r.rows) {
    match_index.emplace(row.match_id, match_index.size());
    auto [it, fresh] = sortable.emplace(row.match_id, true);
    if (row.pt <= 0) it->second = false;
  }

  // sort key: (which match, pt) - or (which match, original position) for a
  // match we can't sort
  std::vector<std::size_t> order(r.rows.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  auto key = [&](std::size_t i) {
    const auto& row = r.rows[i];
    const std::size_t within = sortable[row.match_id] ? static_cast<std::size_t>(row.pt) : i;
    return std::make_pair(match_index[row.match_id], within);
  };
  std::stable_sort(order.begin(), order.end(),
                   [&](std::size_t a, std::size_t b) { return key(a) < key(b); });

  std::unordered_map<std::string, bool> moved;
  std::vector<PointRow> sorted;
  sorted.reserve(r.rows.size());
  for (std::size_t pos = 0; pos < order.size(); ++pos) {
    if (order[pos] != pos) moved[r.rows[order[pos]].match_id] = true;
    sorted.push_back(std::move(r.rows[order[pos]]));
  }
  r.rows = std::move(sorted);
  r.matches_reordered = static_cast<int>(moved.size());
}

}  // namespace

ReadResult read_points_csv(std::istream& in) {
  ReadResult out;

  std::string header_line;
  if (!std::getline(in, header_line)) {
    out.errors.push_back("empty file, no header");
    return out;
  }

  const auto header = split_csv_line(header_line);
  std::unordered_map<std::string, std::size_t> idx;
  for (std::size_t i = 0; i < header.size(); ++i) idx[header[i]] = i;

  if (!idx.count("1st")) {
    out.errors.push_back("no '1st' column in the header, can't read this file");
    return out;
  }

  auto field = [&](const std::vector<std::string>& row, const char* name) -> std::string {
    const auto it = idx.find(name);
    if (it == idx.end() || it->second >= row.size()) return {};
    return row[it->second];
  };

  int line_no = 1;
  std::string line;
  while (std::getline(in, line)) {
    ++line_no;
    if (line.empty()) continue;

    const auto row = split_csv_line(line);
    PointRow p;
    p.line_no = line_no;
    p.match_id = field(row, "match_id");
    p.pts = field(row, "Pts");
    p.first = field(row, "1st");
    p.second = field(row, "2nd");
    p.notes = field(row, "Notes");
    p.tb_set = field(row, "TbSet") == "True";

    auto want_int = [&](const char* name, int& dest) {
      const std::string s = field(row, name);
      if (s.empty()) return;
      if (auto v = to_int(s)) {
        dest = *v;
      } else {
        out.errors.push_back("line " + std::to_string(line_no) + ": '" + name +
                              "' isn't a number ('" + s + "')");
      }
    };
    want_int("Pt", p.pt);
    want_int("Set1", p.set1);
    want_int("Set2", p.set2);
    want_int("Gm1", p.gm1);
    want_int("Gm2", p.gm2);
    want_int("Gm#", p.gm_num);
    want_int("Svr", p.server);
    want_int("PtWinner", p.pt_winner);

    out.rows.push_back(std::move(p));
  }

  sort_into_point_order(out);
  return out;
}

}  // namespace tcl::match
