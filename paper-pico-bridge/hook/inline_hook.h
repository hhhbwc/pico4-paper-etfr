#pragma once
#include <cstddef>
#include <cstdint>
namespace paper_bridge {
struct InlineHook {
  uintptr_t target=0, replacement=0, trampoline=0;
  size_t patch_size=0;
  uint8_t original[16]{};
  uint8_t installed_patch[16]{};
  bool installed=false;
};
bool InstallInlineHook(InlineHook* hook, uintptr_t target, uintptr_t replacement,
                       const uint8_t* expected, size_t expected_size);
bool RemoveInlineHook(InlineHook* hook);
}
