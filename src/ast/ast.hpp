// The tree a charting string parses into. Plain data, no methods that do work.
//
// One Point is one serve and the rally that followed it. Second serves / double
// faults live one level up (the MCP csv has separate 1st and 2nd columns), so a
// Point is always "serve, then rally, then how it ended".

#ifndef TCL_AST_AST_HPP
#define TCL_AST_AST_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace tcl::ast {

enum class Ender { kWinner, kUnforcedError, kForcedError };
enum class ErrorLoc { kNet, kWide, kDeep, kWideDeep };

const char* ender_name(Ender e);
const char* error_loc_name(ErrorLoc l);

struct Serve {
  std::optional<int> direction; // 4 wide, 5 body, 6 down the T
  std::size_t offset = 0;
};

struct Shot {
  char type = '?';              // f b r s v z o p l u y h i
  std::optional<int> direction; // 1 / 2 / 3
  std::optional<int> depth;     // 7 / 8 / 9 (on the return)
  std::optional<char> position; // + approach, - at net, = at baseline
  std::size_t offset = 0;
};

struct Outcome {
  Ender ender = Ender::kWinner;
  std::optional<ErrorLoc> where; // only meaningful for @ / #
  std::size_t offset = 0;
};

struct Point {
  Serve serve;
  std::vector<Shot> rally;
  std::optional<Outcome> outcome; // missing if the string was cut short
};

// A readable dump of the tree, one node per line. Used by `tcl parse` and tests.
std::string to_string(const Point& p);

}  // namespace tcl::ast

#endif  // TCL_AST_AST_HPP
