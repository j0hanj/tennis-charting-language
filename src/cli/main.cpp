// tcl - command line entry point.
//
// so far: --version, `score` (replay point winners), `lex` (token dump), and
// `parse` (dump the parsed tree for a charting string). lint/stats/viz later.

#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "scoring/score.hpp"

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
        "  --version, -v            print version\n"
        "  --help, -h               this message\n"
        "\n"
        "planned: lint, stats, viz\n";
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

  const auto result = tcl::parser::parse(join_args(argc, argv));
  if (result.point) {
    std::cout << tcl::ast::to_string(*result.point);
  }
  for (const auto& d : result.diagnostics) {
    std::cerr << "  at " << d.offset << ": " << d.message << '\n';
  }
  return result.ok() ? 0 : 1;
}

int run_lex(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "lex: give me a charting string, e.g. tcl lex 4ffbbf*\n";
    return 2;
  }

  const auto lexed = tcl::lexer::lex(join_args(argc, argv));
  for (const auto& t : lexed.tokens) {
    std::cout << t.offset << '\t' << tcl::lexer::kind_name(t.kind);
    if (!t.text.empty()) std::cout << '\t' << t.text;
    if (t.kind == tcl::lexer::Kind::kDigit) std::cout << "  (=" << t.value << ')';
    std::cout << '\n';
  }

  for (const auto& d : lexed.diagnostics) {
    std::cerr << "  at " << d.offset << ": " << d.message << '\n';
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

  std::cerr << "tcl: unknown command '" << cmd << "'\n";
  print_usage(std::cerr);
  return 2;
}
