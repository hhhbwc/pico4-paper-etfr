#pragma once
#include <cstdint>

namespace paper_bridge {
constexpr uint32_t kSampleMagic = 0x50475331u; // PGS1
constexpr uint16_t kSampleVersion = 1;

enum SampleFlags : uint32_t {
  kLeftValid = 1u << 0,
  kRightValid = 1u << 1,
  kFusedValid = 1u << 2,
  kCalibrated = 1u << 3,
  kDaemonHealthy = 1u << 4,
};

struct GazeSample {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint64_t sequence;
  uint64_t monotonic_ns;
  uint32_t flags;
  float left_xyz[3];
  float right_xyz[3];
  float fused_xyz[3];
  float left_confidence;
  float right_confidence;
  float fused_confidence;
  uint32_t source_state;
  uint32_t reserved;
};
static_assert(sizeof(GazeSample) == 88, "ABI size changed");
}
