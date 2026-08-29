#pragma once
#include "eye_output_adapter.h"
#include "shared_sample.h"
#include <cstddef>
#include <cstdint>

namespace paper_bridge {

// AArch64-compatible C++ member entry point after the implicit this argument.
// The return value is the int status observed in w0 on the verified build.
using EyeTrackingDataFn = int (*)(void* self, long timestamp, int mode,
                                  void* output);

// Calls the original function unless a fresh, validated shared sample can be
// applied to the caller-owned output object.
int DispatchEyeTrackingData(EyeTrackingDataFn original, void* self,
                            long timestamp, int mode, void* output,
                            size_t output_size, const SharedSample* shared,
                            uint64_t now_ns, uint64_t max_age_ns,
                            const EyeOutputLayout& layout);

// Active Runtime installation remains deliberately disabled.
bool TryInstallEyeHook();
}
