#include "eye_output_adapter.h"
#include <cmath>
#include <cstring>
namespace paper_bridge {
static bool range_ok(size_t off, size_t bytes, size_t total) { return off <= total && bytes <= total - off; }
bool ApplyFusedGaze(void* output, size_t output_size, const GazeSample& s, const EyeOutputLayout& l) {
  if (!output || output_size < l.object_size || l.object_size == 0 ||
      !(s.flags & kFusedValid) || !(s.flags & kCalibrated) ||
      !std::isfinite(s.fused_xyz[0]) || !std::isfinite(s.fused_xyz[1]) || !std::isfinite(s.fused_xyz[2]) ||
      !range_ok(l.valid_offset, 1, output_size) || !range_ok(l.x_offset, 4, output_size) ||
      !range_ok(l.y_offset, 4, output_size) || !range_ok(l.z_offset, 4, output_size)) return false;
  auto* p = static_cast<uint8_t*>(output); const uint8_t valid = 1;
  std::memcpy(p + l.valid_offset, &valid, 1);
  std::memcpy(p + l.x_offset, &s.fused_xyz[0], 4);
  std::memcpy(p + l.y_offset, &s.fused_xyz[1], 4);
  std::memcpy(p + l.z_offset, &s.fused_xyz[2], 4);
  return true;
}
}
