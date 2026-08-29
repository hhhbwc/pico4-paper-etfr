#pragma once
#include "gaze_sample.h"
#include <cstddef>
#include <cstdint>
namespace paper_bridge {
struct EyeOutputLayout {
  size_t valid_offset;
  size_t x_offset;
  size_t y_offset;
  size_t z_offset;
  size_t object_size;
  const uint8_t* prologue;
  size_t prologue_size;
};
// Applies only to a caller-provided, already validated output object.
bool ApplyFusedGaze(void* output, size_t output_size, const GazeSample& sample,
                    const EyeOutputLayout& layout);
}
