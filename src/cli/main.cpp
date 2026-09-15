// tcl - command line entry point.
//
// so far: --version, `score` (replay point winners), `lex` (token dump),
// `parse` (dump the parsed tree), `viz` (draw the point as an svg), `points`
// (parse every point in a csv), `lint` (points + check the score against the
// file). stats still to come.

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "lexer/lexer.hpp"
#include "lexer/render.hpp"
#include "match/reader.hpp"
#include "parser/parser.hpp"
#include "scoring/score.hpp"
#include "sema/reconcile.hpp"
#include "viz/court.hpp"

namespace {

constexpr std::string_view kVersion = "0.0.0";

int print_usage(std::ostream& os) {
  os << "usage: tcl <command> [args]\n"
        "\n"
        "commands:\n"
        "  score <points> [--tb] [--bo5]\n"
        "                           replay points (a = player 1, b = player 2)\n"
        "                           and print the score after each one. --tb\n"
        "                           adds a tiebreak at 6-6, --bo5 is best of five\n"
        "  lex <string>             dump the tokens for a charting string\n"
        "  parse <string>           parse a charting string and print the tree\n"
        "  viz <string> [-o file]   draw the point as an svg (stdout by default)\n"
        "  points <file.csv>        read a match-charting-project points csv,\n"
        "                           run every point through the parser\n"
        "  lint <file.csv>          points, plus replay PtWinner and check the\n"
        "                           score against the file's own Pts column\n"
        "  matchviz <file.csv> [-o file]\n"
        "                           draw every shot in the file on one court\n"
        "  --version, -v            print version\n"
        "  --help, -h               this message\n"
        "\n"
        "planned: stats\n";
  return 0;
}

int run_points(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "points: give me a csv file, e.g. tcl points match.csv\n";
    return 2;
  }

  std::ifstream file(argv[2]);
  if (!file) {
    std::cerr << "points: can't open " << argv[2] << '\n';
    return 2;
  }

  const auto result = tcl::match::read_points_csv(file);
  for (const auto& e : result.errors) {
    std::cerr << "  " << e << '\n';
  }

  int parsed = 0;
  int with_problems = 0;
  for (const auto& row : result.rows) {
    // if the first serve faulted, the real point is in "2nd"
    const std::string& src = !row.second.empty() ? row.second : row.first;
    if (src.empty()) continue;
    ++parsed;
    if (!tcl::parser::parse(src).ok()) ++with_problems;
  }

  std::cout << result.rows.size() << " rows, " << parsed << " points parsed, "
            << with_problems << " the parser had something to say about\n";
  return result.errors.empty() ? 0 : 1;
}

int run_lint(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "lint: give me a csv file, e.g. tcl lint match.csv\n";
    return 2;
  }

  std::ifstream file(argv[2]);
  if (!file) {
    std::cerr << "lint: can't open " << argv[2] << '\n';
    return 2;
  }

  const auto result = tcl::match::read_points_csv(file);
  for (const auto& e : result.errors) {
    std::cerr << "  " << e << '\n';
  }

  int parse_problems = 0;
  for (const auto& row : result.rows) {
    const std::string& src = !row.second.empty() ? row.second : row.first;
    if (src.empty()) continue;
    const auto pr = tcl::parser::parse(src);
    if (!pr.ok()) {
      ++parse_problems;
      std::cerr << row.match_id << " pt " << row.pt << " (line " << row.line_no << "):\n"
                << tcl::lexer::render_diagnostics(src, pr.diagnostics) << "\n\n";
    }
  }

  const auto rec = tcl::sema::reconcile_score(result.rows);
  for (const auto& issue : rec.issues) {
    std::cerr << issue.match_id << " pt " << issue.pt << " (line " << issue.line_no
               << "): " << issue.message;
    if (!issue.expected.empty() || !issue.got.empty()) {
      std::cerr << " (expected " << issue.expected << ", file says " << issue.got << ")";
    }
    std::cerr << '\n';
  }

  std::cout << result.rows.size() << " rows, " << parse_problems << " parse problems, "
            << rec.points_checked << " scores checked, " << rec.issues.size()
            << " score issues\n";

  return (parse_problems == 0 && rec.issues.empty() && result.errors.empty()) ? 0 : 1;
}

int run_viz(int argc, char** argv) {
  std::string src;
  std::string out_path;
  for (int i = 2; i < argc; ++i) {
    const std::string_view a = argv[i];
    if ((a == "-o" || a == "--out") && i + 1 < argc) {
      out_path = argv[++i];
    } else {
      if (!src.empty()) src += ' ';
      src += argv[i];
    }
  }

  if (src.empty()) {
    std::cerr << "viz: give me a charting string, e.g. tcl viz 4ffbbf* -o point.svg\n";
    return 2;
  }

  const auto result = tcl::parser::parse(src);
  if (!result.diagnostics.empty()) {
    std::cerr << tcl::lexer::render_diagnostics(src, result.diagnostics) << '\n';
  }
  if (!result.point) return 1;

  const std::string svg = tcl::viz::render_svg(*result.point, src);
  if (out_path.empty()) {
    std::cout << svg;
  } else {
    std::ofstream file(out_path);
    if (!file) {
      std::cerr << "viz: can't write " << out_path << '\n';
      return 2;
    }
    file << svg;
    std::cerr << "wrote " << out_path << '\n';
  }
  return 0;
}

int run_matchviz(int argc, char** argv) {
  std::string in_path;
  std::string out_path;
  for (int i = 2; i < argc; ++i) {
    const std::string_view a = argv[i];
    if ((a == "-o" || a == "--out") && i + 1 < argc) {
      out_path = argv[++i];
    } else if (in_path.empty()) {
      in_path = argv[i];
    }
  }

  if (in_path.empty()) {
    std::cerr << "matchviz: give me a csv file, e.g. tcl matchviz match.csv -o shots.svg\n";
    return 2;
  }

  std::ifstream file(in_path);
  if (!file) {
    std::cerr << "matchviz: can't open " << in_path << '\n';
    return 2;
  }

  const auto result = tcl::match::read_points_csv(file);
  for (const auto& e : result.errors) {
    std::cerr << "  " << e << '\n';
  }

  std::vector<tcl::ast::Point> points;
  for (const auto& row : result.rows) {
    const std::string& src = !row.second.empty() ? row.second : row.first;
    if (src.empty()) continue;
    if (auto pr = tcl::parser::parse(src); pr.point) points.push_back(std::move(*pr.point));
  }

  if (points.empty()) {
    std::cerr << "matchviz: nothing parsed out of " << in_path << '\n';
    return 1;
  }

  const std::string title =
      result.rows.empty() || result.rows.front().match_id.empty()
          ? in_path
          : result.rows.front().match_id;
  const std::string svg = tcl::viz::render_match_svg(points, title);
  if (out_path.empty()) {
    std::cout << svg;
  } else {
    std::ofstream out(out_path);
    if (!out) {
      std::cerr << "matchviz: can't write " << out_path << '\n';
      return 2;
    }
    out << svg;
    std::cerr << "wrote " << out_path << " from " << points.size() << " points\n";
  }
  return 0;
}

std::string join_args(int argc, char** argv) {
  std::string s;
  for (int i = 2; i < argc; ++i) {
    if (i > 2) s += ' ';
    s += argv[i];
  }
  return s;
}

int run_parse(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "parse: give me a charting string, e.g. tcl parse 4ffbbf*\n";
    return 2;
  }

  const std::string src = join_args(argc, argv);
  const auto result = tcl::parser::parse(src);
  if (result.point) {
    std::cout << tcl::ast::to_string(*result.point);
  }
  if (!result.diagnostics.empty()) {
    std::cerr << '\n' << tcl::lexer::render_diagnostics(src, result.diagnostics) << '\n';
  }
  return result.ok() ? 0 : 1;
}

int run_lex(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "lex: give me a charting string, e.g. tcl lex 4ffbbf*\n";
    return 2;
  }

  const std::string src = join_args(argc, argv);
  const auto lexed = tcl::lexer::lex(src);
  for (const auto& t : lexed.tokens) {
    std::cout << t.offset << '\t' << tcl::lexer::kind_name(t.kind);
    if (!t.text.empty()) std::cout << '\t' << t.text;
    if (t.kind == tcl::lexer::Kind::kDigit) std::cout << "  (=" << t.value << ')';
    std::cout << '\n';
  }

  if (!lexed.diagnostics.empty()) {
    std::cerr << '\n' << tcl::lexer::render_diagnostics(src, lexed.diagnostics) << '\n';
  }
  return lexed.ok() ? 0 : 1;
}

int run_score(int argc, char** argv) {
  std::string points;
  bool tiebreak = false;
  bool bo5 = false;
  for (int i = 2; i < argc; ++i) {
    const std::string_view a = argv[i];
    if (a == "--tb") {
      tiebreak = true;
    } else if (a == "--bo5") {
      bo5 = true;
    } else {
      points += a;
    }
  }

  if (points.empty()) {
    std::cerr << "score: give me some points, e.g. tcl score aabba\n";
    return 2;
  }

  using tcl::scoring::MatchFormat;
  MatchFormat fmt;
  if (bo5) {
    fmt = tiebreak ? MatchFormat::best_of_five_with_tiebreak()
                   : MatchFormat::best_of_five_advantage_set();
  } else {
    fmt = tiebreak ? MatchFormat::best_of_three_with_tiebreak()
                   : MatchFormat::best_of_three_advantage_set();
  }

  tcl::scoring::Score s;
  for (const char c : points) {
    const char lc = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (lc != 'a' && lc != 'b') {
      std::cerr << "score: don't know what '" << c << "' means, use a or b\n";
      return 2;
    }
    if (s.finished) {
      std::cerr << "score: match already over, ignoring the rest\n";
      break;
    }
    s = step(s, lc == 'a' ? tcl::scoring::Player::kOne : tcl::scoring::Player::kTwo, fmt);
    std::cout << scoreline(s, fmt) << '\n';
  }

  std::cout << "\n" << describe(s, fmt) << '\n';
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    print_usage(std::cerr);
    return 2;
  }

  const std::string_view cmd = argv[1];
  if (cmd == "--version" || cmd == "-v") {
    std::cout << "tcl v" << kVersion << '\n';
    return 0;
  }
  if (cmd == "--help" || cmd == "-h") {
    return print_usage(std::cout);
  }
  if (cmd == "score") {
    return run_score(argc, argv);
  }
  if (cmd == "lex") {
    return run_lex(argc, argv);
  }
  if (cmd == "parse") {
    return run_parse(argc, argv);
  }
  if (cmd == "viz") {
    return run_viz(argc, argv);
  }
  if (cmd == "matchviz") {
    return run_matchviz(argc, argv);
  }
  if (cmd == "points") {
    return run_points(argc, argv);
  }
  if (cmd == "lint") {
    return run_lint(argc, argv);
  }

  std::cerr << "tcl: unknown command '" << cmd << "'\n";
  print_usage(std::cerr);
  return 2;
}
