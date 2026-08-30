#include "calibration_fit.h"
#include <cmath>
namespace paper_bridge {
namespace {
bool finite_point(Point p) { return std::isfinite(p.x) && std::isfinite(p.y); }
bool finish_fit(const std::array<Point,9>& in, const std::array<Point,9>& out,
                Calibration* result, float* rmse) {
  if (!result || !result->Fit(in, out)) return false;
  double error = 0;
  for (size_t i = 0; i < 9; ++i) {
    Point mapped = result->Map(in[i]);
    if (!finite_point(mapped)) return false;
    const double dx = mapped.x - out[i].x;
    const double dy = mapped.y - out[i].y;
    error += dx * dx + dy * dy;
  }
  const float value = static_cast<float>(std::sqrt(error / 9.0));
  if (!std::isfinite(value)) return false;
  if (rmse) *rmse = value;
  return true;
}
}

bool FitNinePointCalibration(const std::array<LabeledObservation, 9>& observations,
                             Calibration* result, float* rmse) {
  std::array<Point,9> in{}, out{};
  for (size_t i = 0; i < observations.size(); ++i) {
    const auto& observation = observations[i];
    if (!observation.valid || !finite_point(observation.pupil) ||
        !finite_point(observation.target)) return false;
    in[i] = observation.pupil;
    out[i] = observation.target;
  }
  return finish_fit(in, out, result, rmse);
}

bool FitCalibration(const std::vector<LabeledObservation>& obs, Calibration* result, float* rmse) {
  if (!result || obs.size() < 3) return false;
  std::array<Point,9> in{}, out{}; size_t n=0;
  for (const auto& o:obs) if(o.valid && n<9 && finite_point(o.pupil) && finite_point(o.target)){in[n]=o.pupil;out[n]=o.target;++n;}
  if(n<3) return false; for(size_t i=n;i<9;++i){in[i]=in[n-1];out[i]=out[n-1];}
  return finish_fit(in, out, result, rmse);
}
}
