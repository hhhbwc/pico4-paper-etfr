#pragma once
#include "calibration.h"
#include <array>
namespace paper_bridge {
class CalibrationStore {
 public:
  static bool Load(const char* path, Calibration* left, Calibration* right);
  static bool Save(const char* path, const std::array<Point,9>& left_in,
                   const std::array<Point,9>& right_in,
                   const std::array<Point,9>& output);
};
}
