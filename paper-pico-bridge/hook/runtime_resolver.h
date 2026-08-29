#pragma once
#include <cstdint>
namespace paper_bridge {
struct RuntimeTarget { uintptr_t address=0; bool verified=false; };
RuntimeTarget ResolveKnownEyeClient(const char* maps_path, const uint8_t* expected_prologue, size_t prologue_size);
}
