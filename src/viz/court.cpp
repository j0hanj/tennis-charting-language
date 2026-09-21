#include "viz/court.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

namespace tcl::viz {

namespace {

constexpr double kPxPerFoot = 9.0;
constexpr double kMargin = 34.0;
constexpr double kTitleBand = 30.0;
constexpr double kCourtW = 27.0 * kPxPerFoot; // singles width
constexpr double kCourtH = 78.0 * kPxPerFoot; // baseline to baseline

struct Pt {
  double x = 0.0;
  double y = 0.0;
};

struct Court {
  double x0 = kMargin;
  double y0 = kMargin + kTitleBand;
  double w = kCourtW;
  double h = kCourtH;

  double net_y() const { return y0 + h / 2.0; }
  double center_x() const { return x0 + w / 2.0; }
  double service_offset() const { return 21.0 * kPxPerFoot; } // service line from net
  double half_len() const { return h / 2.0; }
  double frac_x(double f) const { return x0 + f * w; }
};

std::string f(double v) {
  std::ostringstream o;
  o << std::fixed << std::setprecision(1) << v;
  return o.str();
}

void draw_court(std::ostringstream& s, const Court& c) {
  const double left = c.x0;
  const double right = c.x0 + c.w;
  const double top = c.y0;
  const double bot = c.y0 + c.h;
  const double net = c.net_y();
  const double tsl = net - c.service_offset(); // top service line
  const double bsl = net + c.service_offset(); // bottom service line

  // playing surface
  s << "  <rect x='" << f(left) << "' y='" << f(top) << "' width='" << f(c.w)
    << "' height='" << f(c.h) << "' fill='#3d7a4f' stroke='#f4f4f4'"
    << " stroke-width='2'/>\n";

  auto line = [&](double x1, double y1, double x2, double y2, double sw) {
    s << "  <line x1='" << f(x1) << "' y1='" << f(y1) << "' x2='" << f(x2)
      << "' y2='" << f(y2) << "' stroke='#f4f4f4' stroke-width='" << f(sw)
      << "'/>\n";
  };

  line(left, tsl, right, tsl, 1.5);              // top service line
  line(left, bsl, right, bsl, 1.5);              // bottom service line
  line(c.center_x(), tsl, c.center_x(), bsl, 1.5); // center service line
  line(c.center_x(), top, c.center_x(), top + 6, 1.5);  // center marks
  line(c.center_x(), bot - 6, c.center_x(), bot, 1.5);
  line(left, net, right, net, 3.0);              // net
}

double dir_frac(const tcl::ast::Shot& shot, double fallback) {
  if (!shot.direction) return fallback;
  switch (*shot.direction) {
    case 1: return 0.22;
    case 2: return 0.50;
    case 3: return 0.78;
    default: return 0.50;
  }
}

double depth_frac(const tcl::ast::Shot& shot) {
  if (!shot.depth) return 0.78;
  switch (*shot.depth) {
    case 7: return 0.45; // shallow
    case 8: return 0.70;
    case 9: return 0.94; // very deep
    default: return 0.78;
  }
}

std::vector<Pt> ball_path(const tcl::ast::Point& p, const Court& c) {
  std::vector<Pt> pts;

  // server starts on the bottom baseline, deuce side
  pts.push_back({c.frac_x(0.62), c.y0 + c.h - 5.0});

  // serve into the top service box (kept on the left half for a first point)
  double serve_f = 0.34;
  if (p.serve.direction) {
    switch (*p.serve.direction) {
      case 4: serve_f = 0.12; break; // wide
      case 5: serve_f = 0.30; break; // body
      case 6: serve_f = 0.46; break; // down the T
      default: break;
    }
  }
  pts.push_back({c.frac_x(serve_f), c.net_y() - c.service_offset() * 0.5});

  double wander = 0.0;
  for (std::size_t k = 0; k < p.rally.size(); ++k) {
    const bool lands_bottom = (k % 2 == 0); // the return lands on the server's end
    double fx = dir_frac(p.rally[k], 0.0);
    if (fx == 0.0) {
      wander = (wander == 0.0) ? 0.14 : -wander;
      fx = 0.5 + wander;
    }
    double d = depth_frac(p.rally[k]);
    if (d > 0.97) d = 0.97;
    const double y = lands_bottom ? c.net_y() + c.half_len() * d
                                  : c.net_y() - c.half_len() * d;
    pts.push_back({c.frac_x(fx), y});
  }

  return pts;
}

// Everything render_svg and render_match_svg both need: the ball path plus
// where the point actually ended up and how.
struct PointRender {
  std::vector<Pt> pts;
  bool is_winner = true;
  std::string tag = "in play";
};

PointRender compute_point_render(const tcl::ast::Point& point, const Court& c) {
  PointRender out;
  out.pts = ball_path(point, c);

  // a missed serve: the path stops where it landed, marked as a miss
  if (point.fault) {
    out.is_winner = false;
    out.tag = std::string("fault (") + tcl::ast::error_loc_name(*point.fault) + ")";
    return out;
  }
  if (!point.outcome) return out;

  using tcl::ast::Ender;
  using tcl::ast::ErrorLoc;
  const auto& o = *point.outcome;
  out.is_winner = (o.ender == Ender::kWinner);
  out.tag = tcl::ast::ender_name(o.ender);
  if (out.is_winner) return out;

  const Pt last = out.pts.back();
  const ErrorLoc where = o.where.value_or(ErrorLoc::kNet);
  Pt e = last;
  const bool right_side = last.x > c.center_x();
  const bool top_side = last.y < c.net_y();
  switch (where) {
    case ErrorLoc::kNet:
      e = {last.x, c.net_y()};
      break;
    case ErrorLoc::kWide:
      e = {right_side ? c.x0 + c.w + 16.0 : c.x0 - 16.0, last.y};
      break;
    case ErrorLoc::kDeep:
      e = {last.x, top_side ? c.y0 - 16.0 : c.y0 + c.h + 16.0};
      break;
    case ErrorLoc::kWideDeep:
      e = {right_side ? c.x0 + c.w + 16.0 : c.x0 - 16.0,
           top_side ? c.y0 - 16.0 : c.y0 + c.h + 16.0};
      break;
  }
  out.tag += " (";
  out.tag += tcl::ast::error_loc_name(where);
  out.tag += ")";
  out.pts.push_back(e);
  return out;
}

}  // namespace

std::string render_svg(const tcl::ast::Point& point, std::string_view source) {
  const Court c;
  const double svg_w = kCourtW + 2 * kMargin;
  const double svg_h = kCourtH + 2 * kMargin + kTitleBand;

  const PointRender pr = compute_point_render(point, c);
  const std::vector<Pt>& pts = pr.pts;
  const bool is_winner = pr.is_winner;
  const std::string& tag = pr.tag;

  std::ostringstream s;
  s << "<svg xmlns='http://www.w3.org/2000/svg' width='" << f(svg_w)
    << "' height='" << f(svg_h) << "' viewBox='0 0 " << f(svg_w) << ' '
    << f(svg_h) << "' font-family='ui-monospace, Menlo, Consolas, monospace'>\n";
  s << "  <rect width='100%' height='100%' fill='#20242b'/>\n";

  s << "  <text x='" << f(kMargin) << "' y='24' fill='#f4f4f4' font-size='15'>"
    << source << "</text>\n";
  s << "  <text x='" << f(svg_w - kMargin)
    << "' y='24' fill='#9aa4b2' font-size='12' text-anchor='end'>" << tag
    << "</text>\n";

  draw_court(s, c);

  // ball path
  s << "  <polyline fill='none' stroke='#f4c542' stroke-width='2.4'"
    << " stroke-linejoin='round' stroke-linecap='round' points='";
  for (std::size_t i = 0; i < pts.size(); ++i) {
    s << f(pts[i].x) << ',' << f(pts[i].y);
    if (i + 1 < pts.size()) s << ' ';
  }
  s << "'/>\n";

  // bounces
  for (std::size_t i = 1; i + 1 < pts.size(); ++i) {
    s << "  <circle cx='" << f(pts[i].x) << "' cy='" << f(pts[i].y)
      << "' r='3' fill='#f4f4f4'/>\n";
  }

  // serve contact
  s << "  <circle cx='" << f(pts.front().x) << "' cy='" << f(pts.front().y)
    << "' r='5' fill='#3aa0ff'/>\n";

  // where it finished
  const Pt end = pts.back();
  if (is_winner) {
    s << "  <circle cx='" << f(end.x) << "' cy='" << f(end.y)
      << "' r='6' fill='#f4c542' stroke='#20242b' stroke-width='1.5'/>\n";
  } else {
    s << "  <circle cx='" << f(end.x) << "' cy='" << f(end.y)
      << "' r='6' fill='#e5484d'/>\n";
    s << "  <path d='M" << f(end.x - 4) << ' ' << f(end.y - 4) << 'L'
      << f(end.x + 4) << ' ' << f(end.y + 4) << 'M' << f(end.x + 4) << ' '
      << f(end.y - 4) << 'L' << f(end.x - 4) << ' ' << f(end.y + 4)
      << "' stroke='#20242b' stroke-width='1.5'/>\n";
  }

  s << "</svg>\n";
  return s.str();
}

std::string render_match_svg(const std::vector<tcl::ast::Point>& points, std::string_view title) {
  const Court c;
  const double svg_w = kCourtW + 2 * kMargin;
  const double svg_h = kCourtH + 2 * kMargin + kTitleBand;

  std::vector<PointRender> renders;
  renders.reserve(points.size());
  int winners = 0;
  int errors = 0;
  for (const auto& p : points) {
    renders.push_back(compute_point_render(p, c));
    if (renders.back().is_winner) {
      ++winners;
    } else {
      ++errors;
    }
  }

  // the svg is only ~310px wide - a long match_id or file path would run
  // straight off the edge, so clip it
  constexpr std::size_t kMaxTitle = 30;
  std::string shown_title(title);
  if (shown_title.size() > kMaxTitle) {
    shown_title = shown_title.substr(0, kMaxTitle - 1) + "…";
  }

  std::ostringstream s;
  s << "<svg xmlns='http://www.w3.org/2000/svg' width='" << f(svg_w)
    << "' height='" << f(svg_h) << "' viewBox='0 0 " << f(svg_w) << ' '
    << f(svg_h) << "' font-family='ui-monospace, Menlo, Consolas, monospace'>\n";
  s << "  <rect width='100%' height='100%' fill='#20242b'/>\n";

  s << "  <text x='" << f(kMargin) << "' y='22' fill='#f4f4f4' font-size='12'>" << shown_title
    << "</text>\n";
  s << "  <text x='" << f(kMargin) << "' y='" << f(kMargin - 4) << "' fill='#9aa4b2'"
    << " font-size='11'>" << points.size() << " points, " << winners << " winners, " << errors
    << " errors</text>\n";

  draw_court(s, c);

  // the direction/depth codes only give ~9 rough spots per side, so with
  // hundreds of shots stacking exactly on top of each other reads as a grid,
  // not a shot chart. nudge each dot a little - deterministic, seeded off its
  // own point/shot index, so the same file always draws the same picture
  auto jitter = [](std::size_t seed) {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return static_cast<double>(seed % 2000) / 1000.0 - 1.0; // -1..1
  };

  // every bounce from every point, translucent so it builds up where shots
  // actually cluster - the closest thing to a real shot chart this schematic
  // model can produce (no ball tracking, just what the notation implies)
  for (std::size_t pi = 0; pi < renders.size(); ++pi) {
    const auto& pr = renders[pi];
    for (std::size_t i = 1; i + 1 < pr.pts.size(); ++i) {
      const double jx = jitter(pi * 131 + i * 7) * 7.0;
      const double jy = jitter(pi * 131 + i * 7 + 91) * 7.0;
      s << "  <circle cx='" << f(pr.pts[i].x + jx) << "' cy='" << f(pr.pts[i].y + jy)
        << "' r='2.5' fill='#f4f4f4' fill-opacity='0.16'/>\n";
    }
    const double ejx = jitter(pi * 131 + 991) * 5.0;
    const double ejy = jitter(pi * 131 + 997) * 5.0;
    const Pt end{pr.pts.back().x + ejx, pr.pts.back().y + ejy};
    const char* color = pr.is_winner ? "#f4c542" : "#e5484d";
    s << "  <circle cx='" << f(end.x) << "' cy='" << f(end.y) << "' r='3.5' fill='" << color
      << "' fill-opacity='0.55'/>\n";
  }

  s << "</svg>\n";
  return s.str();
}

}  // namespace tcl::viz
