#include "eye_hook_controller.h"
#include "runtime_probe.h"
#include "runtime_signature.h"
#include <ctime>

namespace paper_bridge {

int DispatchEyeTrackingData(EyeTrackingDataFn original, void* self,
                            long timestamp, int mode, void* output,
                            size_t output_size, const SharedSample* shared,
                            uint64_t now_ns, uint64_t max_age_ns,
                            const EyeOutputLayout& layout) {
  if (!original) return -1;
  GazeSample sample{};
  if (ReadFreshSample(shared, now_ns, max_age_ns, &sample) &&
      ApplyFusedGaze(output, output_size, sample, layout)) {
    return 0;
  }
  return original(self, timestamp, mode, output);
}

// Deliberately disabled: runtime identity, complete eyepose layout, and
// replacement behavior require a dedicated device test before installation.
bool TryInstallEyeHook() { return false; }
}
