#pragma once
#include "calibration.h"
#include <array>
#include <vector>
namespace paper_bridge {
struct LabeledObservation { Point pupil; Point target; bool valid; };
bool FitCalibration(const std::vector<LabeledObservation>& observations,
                    Calibration* result, float* rmse);
}
