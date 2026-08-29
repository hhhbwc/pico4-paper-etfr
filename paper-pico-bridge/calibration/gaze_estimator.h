#pragma once
#include "calibration.h"
#include "pupil_detector.h"
#include "gaze_sample.h"
#include <cstdint>
namespace paper_bridge {
class GazeEstimator {
 public:
  bool FitLeft(const std::array<Point,9>& in, const std::array<Point,9>& out) { return left_.Fit(in,out); }
  bool FitRight(const std::array<Point,9>& in, const std::array<Point,9>& out) { return right_.Fit(in,out); }
  bool LoadCalibration(const char* path);
  bool ready() const { return left_.ready() && right_.ready(); }
  bool Estimate(const PupilObservation& left, const PupilObservation& right, uint64_t monotonic_ns, GazeSample* out) const;
 private:
  Calibration left_, right_;
};
}
