#include "ast/ast.hpp"

namespace tcl::ast {

const char* ender_name(Ender e) {
  switch (e) {
    case Ender::kWinner:        return "winner";
    case Ender::kUnforcedError: return "unforced";
    case Ender::kForcedError:   return "forced";
  }
  return "?";
}

const char* error_loc_name(ErrorLoc l) {
  switch (l) {
    case ErrorLoc::kNet:      return "net";
    case ErrorLoc::kWide:     return "wide";
    case ErrorLoc::kDeep:     return "deep";
    case ErrorLoc::kWideDeep: return "wide+deep";
  }
  return "?";
}

std::string to_string(const Point& p) {
  std::string s = "serve dir=";
  s += p.serve.direction ? std::to_string(*p.serve.direction) : "?";
  s += '\n';

  for (const auto& shot : p.rally) {
    s += "shot ";
    s += shot.type;
    if (shot.direction) s += " dir=" + std::to_string(*shot.direction);
    if (shot.depth) s += " depth=" + std::to_string(*shot.depth);
    if (shot.position) {
      s += " pos=";
      s += *shot.position;
    }
    s += '\n';
  }

  if (p.outcome) {
    s += "end ";
    s += ender_name(p.outcome->ender);
    if (p.outcome->where) {
      s += ' ';
      s += error_loc_name(*p.outcome->where);
    }
    s += '\n';
  } else {
    s += "end (missing)\n";
  }
  return s;
}

}  // namespace tcl::ast
