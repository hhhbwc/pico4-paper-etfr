#pragma once
#include "gaze_sample.h"
#include <atomic>
#include <cstddef>

namespace paper_bridge {
struct SharedSample {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  std::atomic<uint32_t> lock;
  GazeSample sample;
};

void InitializeShared(SharedSample* shared);
void PublishSample(SharedSample* shared, const GazeSample& sample);
bool ReadFreshSample(const SharedSample* shared, uint64_t now_ns,
                     uint64_t max_age_ns, GazeSample* out);
}
