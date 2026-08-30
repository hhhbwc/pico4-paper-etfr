#include "calibration_store.h"
#include <cstdio>
#include <string>
#include <cmath>
namespace paper_bridge {
namespace {
bool finite_points(const std::array<Point, 9>& points) {
  for (const Point& point : points)
    if (!std::isfinite(point.x) || !std::isfinite(point.y)) return false;
  return true;
}
}

bool CalibrationStore::Load(const char* path, Calibration* left, Calibration* right) {
  if (!path || !left || !right) return false;
  FILE* f = std::fopen(path, "rb");
  if (!f) return false;
  if (std::fseek(f, 0, SEEK_END) != 0) { std::fclose(f); return false; }
  long length = std::ftell(f);
  if (length != long(27 * sizeof(Point)) || std::fseek(f, 0, SEEK_SET) != 0) {
    std::fclose(f);
    return false;
  }
  std::array<Point, 9> li{}, ri{}, targets{};
  bool ok = std::fread(li.data(), sizeof(Point), 9, f) == 9 &&
            std::fread(ri.data(), sizeof(Point), 9, f) == 9 &&
            std::fread(targets.data(), sizeof(Point), 9, f) == 9;
  std::fclose(f);
  if (!ok || !finite_points(li) || !finite_points(ri) || !finite_points(targets)) return false;
  return left->Fit(li, targets) && right->Fit(ri, targets);
}

bool CalibrationStore::Save(const char* path, const std::array<Point, 9>& li,
                            const std::array<Point, 9>& ri,
                            const std::array<Point, 9>& targets) {
  if (!path || !finite_points(li) || !finite_points(ri) || !finite_points(targets)) return false;
  Calibration left, right;
  if (!left.Fit(li, targets) || !right.Fit(ri, targets)) return false;
  std::string tmp = std::string(path) + ".tmp";
  FILE* f = std::fopen(tmp.c_str(), "wb");
  if (!f) return false;
  bool ok = std::fwrite(li.data(), sizeof(Point), 9, f) == 9 &&
            std::fwrite(ri.data(), sizeof(Point), 9, f) == 9 &&
            std::fwrite(targets.data(), sizeof(Point), 9, f) == 9;
  std::fflush(f);
  std::fclose(f);
  if (!ok) { std::remove(tmp.c_str()); return false; }
  return std::rename(tmp.c_str(), path) == 0;
}
}
