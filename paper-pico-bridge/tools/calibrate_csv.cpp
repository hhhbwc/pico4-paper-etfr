#include "calibration_fit.h"
#include "calibration_store.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace paper_bridge;

namespace {
struct Observation { Point pupil{}; float confidence=0; bool valid=false; };
struct Accumulator { std::vector<Observation> samples; };
struct Target { Point point{}; bool set=false; };

float median(std::vector<float> values) {
  if (values.empty()) return 0.0f;
  std::sort(values.begin(), values.end());
  const size_t middle = values.size() / 2;
  return values.size() % 2 ? values[middle] : (values[middle - 1] + values[middle]) * 0.5f;
}

bool stable_samples(const Accumulator& a) {
  if (a.samples.size() < 3) return false;
  std::vector<float> xs, ys;
  xs.reserve(a.samples.size()); ys.reserve(a.samples.size());
  for (const auto& sample : a.samples) { xs.push_back(sample.pupil.x); ys.push_back(sample.pupil.y); }
  const float cx = median(xs), cy = median(ys);
  std::vector<float> dx, dy;
  dx.reserve(xs.size()); dy.reserve(ys.size());
  for (size_t i = 0; i < xs.size(); ++i) { dx.push_back(std::fabs(xs[i] - cx)); dy.push_back(std::fabs(ys[i] - cy)); }
  const float mad_x = median(dx), mad_y = median(dy);
  const float scale = std::max(1.0f, std::max(std::fabs(cx), std::fabs(cy)));
  return std::isfinite(mad_x) && std::isfinite(mad_y) && std::max(mad_x, mad_y) <= scale * 0.25f;
}

bool parse_row(const std::string& line, int* id, Point* target, bool* left,
               Point* pupil, float* confidence, bool* valid) {
  std::stringstream ss(line); std::string field; std::string eye;
  std::string values[8];
  for (int i=0; i<8; ++i) if (!std::getline(ss, values[i], ',')) return false;
  try {
    *id = std::stoi(values[0]);
    target->x = std::stof(values[1]); target->y = std::stof(values[2]);
    eye = values[3];
    pupil->x = std::stof(values[4]); pupil->y = std::stof(values[5]);
    *confidence = std::stof(values[6]); *valid = std::stoi(values[7]) != 0;
  } catch (...) { return false; }
  *left = eye == "left";
  if (!*left && eye != "right") return false;
  return id && target && pupil && confidence && valid &&
         *id >= 0 && *id < 9 && std::isfinite(target->x) && std::isfinite(target->y) &&
         std::isfinite(pupil->x) && std::isfinite(pupil->y) &&
         std::isfinite(*confidence) && *confidence >= 0.0f && *confidence <= 1.0f;
}
}

int main(int argc, char** argv) {
  if (argc != 3) { std::cerr << "usage: calibrate_csv <labeled.csv> <output.bin>\n"; return 2; }
  std::ifstream input(argv[1]); if (!input) { std::cerr << "cannot open input\n"; return 3; }
  std::string line; if (!std::getline(input, line) || line.find("target_id") == std::string::npos) { std::cerr << "invalid header\n"; return 4; }
  std::array<Target,9> targets{}; std::array<Accumulator,9> left{}, right{};
  while (std::getline(input, line)) {
    if (line.empty()) continue;
    int id=0; Point target{}, pupil{}; bool is_left=false, valid=false; float confidence=0;
    if (!parse_row(line, &id, &target, &is_left, &pupil, &confidence, &valid)) { std::cerr << "invalid row\n"; return 5; }
    if (!valid || confidence < 0.5f) continue;
    if (targets[id].set && (targets[id].point.x != target.x || targets[id].point.y != target.y)) { std::cerr << "target mismatch\n"; return 6; }
    targets[id] = {target, true};
    Accumulator& a = is_left ? left[id] : right[id];
    a.samples.push_back({pupil, confidence, true});
  }
  std::array<LabeledObservation,9> left_obs{}, right_obs{}; std::array<Point,9> target_points{};
  for (size_t i=0; i<9; ++i) {
    if (!targets[i].set || !stable_samples(left[i]) || !stable_samples(right[i])) { std::cerr << "missing target or stable eye sample\n"; return 7; }
    target_points[i] = targets[i].point;
    auto aggregate = [](const Accumulator& a) {
      std::vector<float> xs, ys; xs.reserve(a.samples.size()); ys.reserve(a.samples.size());
      for (const auto& s : a.samples) { xs.push_back(s.pupil.x); ys.push_back(s.pupil.y); }
      std::sort(xs.begin(), xs.end()); std::sort(ys.begin(), ys.end());
      const size_t trim = xs.size() >= 5 ? xs.size() / 10 : 0;
      double x = 0, y = 0; size_t n = 0;
      for (size_t j = trim; j + trim < xs.size(); ++j) { x += xs[j]; y += ys[j]; ++n; }
      return Point{static_cast<float>(x / n), static_cast<float>(y / n)};
    };
    left_obs[i] = {aggregate(left[i]), target_points[i], true};
    right_obs[i] = {aggregate(right[i]), target_points[i], true};
  }
  Calibration left_fit, right_fit; float left_rmse=0, right_rmse=0;
  if (!FitNinePointCalibration(left_obs, &left_fit, &left_rmse) || !FitNinePointCalibration(right_obs, &right_fit, &right_rmse)) { std::cerr << "fit failed\n"; return 8; }
  const auto direction_ok = [](const Calibration& fit) {
    const Point left = fit.Map({0.0f, 0.5f});
    const Point right = fit.Map({1.0f, 0.5f});
    const Point top = fit.Map({0.5f, 0.0f});
    const Point bottom = fit.Map({0.5f, 1.0f});
    return std::isfinite(left.x) && std::isfinite(right.x) && std::isfinite(top.y) && std::isfinite(bottom.y) &&
           right.x > left.x && bottom.y > top.y;
  };
  if (!direction_ok(left_fit) || !direction_ok(right_fit)) { std::cerr << "calibration direction rejected\n"; return 9; }
  if (left_rmse > 0.15f || right_rmse > 0.15f || !CalibrationStore::Save(argv[2],
      [&] { std::array<Point,9> a{}; for (size_t i=0;i<9;++i) a[i]=left_obs[i].pupil; return a; }(),
      [&] { std::array<Point,9> a{}; for (size_t i=0;i<9;++i) a[i]=right_obs[i].pupil; return a; }(), target_points)) { std::cerr << "calibration rejected\n"; return 9; }
  Calibration check_left, check_right; if (!CalibrationStore::Load(argv[2], &check_left, &check_right)) { std::cerr << "round-trip failed\n"; return 10; }
  std::cout << "calibration written left_rmse=" << left_rmse << " right_rmse=" << right_rmse << "\n";
  return 0;
}
