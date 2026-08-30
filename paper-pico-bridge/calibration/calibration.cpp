#include "calibration.h"
#include <algorithm>
#include <cmath>
namespace paper_bridge {
namespace {
bool finite_point(Point p) { return std::isfinite(p.x) && std::isfinite(p.y); }
}

bool Calibration::Fit(const std::array<Point,9>& in, const std::array<Point,9>& out) {
  ready_ = false;
  double m[8][9]{};
  for (int k = 0; k < 9; ++k) {
    if (!finite_point(in[k]) || !finite_point(out[k])) return false;
    const double x = in[k].x, y = in[k].y, u = out[k].x, v = out[k].y;
    const double rows[2][8] = {
      {x, y, 1.0, 0.0, 0.0, 0.0, -x * u, -y * u},
      {0.0, 0.0, 0.0, x, y, 1.0, -x * v, -y * v},
    };
    const double rhs[2] = {u, v};
    for (int q = 0; q < 2; ++q) {
      for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) m[i][j] += rows[q][i] * rows[q][j];
        m[i][8] += rows[q][i] * rhs[q];
      }
    }
  }
  for (int c = 0; c < 8; ++c) {
    int pivot = c;
    for (int r = c + 1; r < 8; ++r)
      if (std::fabs(m[r][c]) > std::fabs(m[pivot][c])) pivot = r;
    double scale = 0.0;
    for (int r = c; r < 8; ++r) for (int j = c; j < 8; ++j)
      scale = std::max(scale, std::fabs(m[r][j]));
    if (scale == 0.0 || std::fabs(m[pivot][c]) <= scale * 1e-12) return false;
    for (int j = c; j < 9; ++j) std::swap(m[c][j], m[pivot][j]);
    const double d = m[c][c];
    for (int j = c; j < 9; ++j) m[c][j] /= d;
    for (int r = 0; r < 8; ++r) if (r != c) {
      const double f = m[r][c];
      for (int j = c; j < 9; ++j) m[r][j] -= f * m[c][j];
    }
  }
  float candidate[9]{};
  for (int i = 0; i < 8; ++i) candidate[i] = static_cast<float>(m[i][8]);
  candidate[8] = 1.0f;
  for (float value : candidate) if (!std::isfinite(value)) return false;
  for (int k = 0; k < 9; ++k) {
    const double w = candidate[6] * in[k].x + candidate[7] * in[k].y + candidate[8];
    if (!std::isfinite(w) || std::fabs(w) < 1e-8) return false;
    const Point mapped{static_cast<float>((candidate[0] * in[k].x + candidate[1] * in[k].y + candidate[2]) / w),
                       static_cast<float>((candidate[3] * in[k].x + candidate[4] * in[k].y + candidate[5]) / w)};
    if (!finite_point(mapped)) return false;
  }
  std::copy(candidate, candidate + 9, h_);
  ready_ = true;
  return true;
}

Point Calibration::Map(Point p) const {
  if (!ready_ || !finite_point(p)) return {};
  const double w = static_cast<double>(h_[6]) * p.x + h_[7] * p.y + h_[8];
  if (!std::isfinite(w) || std::fabs(w) < 1e-8) return {};
  const double x = (static_cast<double>(h_[0]) * p.x + h_[1] * p.y + h_[2]) / w;
  const double y = (static_cast<double>(h_[3]) * p.x + h_[4] * p.y + h_[5]) / w;
  if (!std::isfinite(x) || !std::isfinite(y)) return {};
  return {static_cast<float>(x), static_cast<float>(y)};
}
}
