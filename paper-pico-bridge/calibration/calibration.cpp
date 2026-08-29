#include "calibration.h"
#include <algorithm>
#include <cmath>
namespace paper_bridge {
bool Calibration::Fit(const std::array<Point,9>& in, const std::array<Point,9>& out) {
  for (int eq = 0; eq < 2; ++eq) {
    float m[3][4]{};
    for (int k = 0; k < 9; ++k) {
      const float r[3] = {in[k].x, in[k].y, 1.0f};
      const float y = eq ? out[k].y : out[k].x;
      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) m[i][j] += r[i] * r[j];
        m[i][3] += r[i] * y;
      }
    }
    for (int c = 0; c < 3; ++c) {
      int pivot = c;
      for (int r = c + 1; r < 3; ++r)
        if (std::fabs(m[r][c]) > std::fabs(m[pivot][c])) pivot = r;
      if (std::fabs(m[pivot][c]) < 1e-6f) return false;
      for (int j = c; j < 4; ++j) std::swap(m[c][j], m[pivot][j]);
      const float d = m[c][c];
      for (int j = c; j < 4; ++j) m[c][j] /= d;
      for (int r = 0; r < 3; ++r) if (r != c) {
        const float f = m[r][c];
        for (int j = c; j < 4; ++j) m[r][j] -= f * m[c][j];
      }
    }
    for (int i = 0; i < 3; ++i) a_[eq * 3 + i] = m[i][3];
  }
  ready_ = true;
  return true;
}
Point Calibration::Map(Point p) const {
  if (!ready_) return {};
  return {a_[0] * p.x + a_[1] * p.y + a_[2],
          a_[3] * p.x + a_[4] * p.y + a_[5]};
}
}
