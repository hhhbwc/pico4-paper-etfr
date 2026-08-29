#include "shared_sample.h"
#include <cmath>
#include <cstring>

namespace paper_bridge {
void InitializeShared(SharedSample* s) {
  std::memset(s, 0, sizeof(*s));
  s->magic = kSampleMagic;
  s->version = kSampleVersion;
  s->size = sizeof(GazeSample);
  s->lock.store(0, std::memory_order_release);
}

void PublishSample(SharedSample* s, const GazeSample& sample) {
  uint32_t seq = s->lock.load(std::memory_order_relaxed);
  if (!(seq & 1u)) ++seq;
  s->lock.store(seq, std::memory_order_release);
  std::memcpy(&s->sample, &sample, sizeof(sample));
  s->lock.store(seq + 1u, std::memory_order_release);
}

static bool finite3(const float* v) {
  return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
}

bool ReadFreshSample(const SharedSample* s, uint64_t now, uint64_t max_age,
                    GazeSample* out) {
  if (!s || !out || s->magic != kSampleMagic || s->version != kSampleVersion ||
      s->size != sizeof(GazeSample)) return false;
  for (int i = 0; i < 3; ++i) {
    uint32_t before = s->lock.load(std::memory_order_acquire);
    if (before & 1u) continue;
    std::memcpy(out, &s->sample, sizeof(*out));
    uint32_t after = s->lock.load(std::memory_order_acquire);
    if (before != after || (after & 1u)) continue;
    if (out->magic != kSampleMagic || out->version != kSampleVersion ||
        out->size != sizeof(GazeSample) || !(out->flags & kDaemonHealthy) ||
        !(out->flags & kCalibrated) || now < out->monotonic_ns ||
        now - out->monotonic_ns > max_age || !finite3(out->fused_xyz)) return false;
    return true;
  }
  return false;
}
}
