#pragma once
#include <array>
namespace paper_bridge {
struct Point { float x, y; };
class Calibration {
 public:
  static constexpr unsigned kModelVersion = 2;
  bool Fit(const std::array<Point, 9>& input, const std::array<Point, 9>& output);
  Point Map(Point p) const;
  bool ready() const { return ready_; }
 private:
  float h_[9]{};
  bool ready_ = false;
};
}
