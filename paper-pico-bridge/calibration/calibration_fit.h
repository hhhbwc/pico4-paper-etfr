#pragma once
#include "calibration.h"
#include <array>
#include <vector>
namespace paper_bridge {
struct LabeledObservation { Point pupil; Point target; bool valid; };
bool FitCalibration(const std::vector<LabeledObservation>& observations,
                    Calibration* result, float* rmse);

// Strict artifact path: exactly one valid observation for each of nine targets.
bool FitNinePointCalibration(const std::array<LabeledObservation, 9>& observations,
                             Calibration* result, float* rmse);
}
