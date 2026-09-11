#include "match/reader.hpp"

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

  return out;
}

}  // namespace tcl::match
