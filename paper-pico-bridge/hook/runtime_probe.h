#pragma once
#include <cstdint>
namespace paper_bridge {
struct RuntimeProbe { uintptr_t base=0; uintptr_t get_eye_tracking_data=0; bool prologue_ok=false; };
RuntimeProbe ProbeSelfRuntime();
}
