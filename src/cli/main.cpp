// tcl - Tennis Charting Language command-line entry point.
//
// For now this only reports its version. Subcommands (lint, parse, stats,
// viz, repl) are added as the library underneath them lands.

#include <iostream>
#include <string_view>

namespace {

constexpr std::string_view kVersion = "0.0.0";

int print_version() {
  std::cout << "tcl v" << kVersion << '\n';
  return 0;
}

int print_usage(std::ostream& os) {
  os << "usage: tcl <command> [args]\n"
        "\n"
        "commands:\n"
        "  --version, -v   print version and exit\n"
        "  --help, -h      print this message\n"
        "\n"
        "planned: lint, parse, stats, viz, repl (not yet implemented)\n";
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    print_usage(std::cerr);
    return 2;
  }

  const std::string_view arg = argv[1];
  if (arg == "--version" || arg == "-v") {
    return print_version();
  }
  if (arg == "--help" || arg == "-h") {
    return print_usage(std::cout);
  }

  std::cerr << "tcl: unknown command '" << arg << "'\n";
  print_usage(std::cerr);
  return 2;
}
