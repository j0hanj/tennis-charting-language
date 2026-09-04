// tcl - command line entry point.
//
// so far: --version, and a little `score` command that replays a string of
// point winners through the scoring engine. lint/parse/stats/viz come later.

#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

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
        "  --version, -v            print version\n"
        "  --help, -h               this message\n"
        "\n"
        "planned: lint, parse, stats, viz\n";
  return 0;
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

  std::cerr << "tcl: unknown command '" << cmd << "'\n";
  print_usage(std::cerr);
  return 2;
}
