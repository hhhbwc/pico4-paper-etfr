#include "runtime_resolver.h"
#include "runtime_signature.h"
#include <cstdio>
#include <cstring>
namespace paper_bridge {
RuntimeTarget ResolveKnownEyeClient(const char* maps_path, const uint8_t* expected, size_t n) {
  if (!maps_path || !expected || n == 0) return {};
  FILE* f = std::fopen(maps_path, "r"); if (!f) return {};
  char line[512]; uintptr_t base = 0; uintptr_t map_end = 0; char path[256];
  while (std::fgets(line, sizeof(line), f)) {
    unsigned long long start=0, end=0, off=0; path[0]=0;
    if (std::sscanf(line, "%llx-%llx %*4s %llx %*s %*s %255s", &start, &end, &off, path) >= 4 &&
        std::strstr(path, "libeyetrackingclient.pxr.so") && off == 0) {
      base = (uintptr_t)start; map_end = (uintptr_t)end; break;
    }
  }
  std::fclose(f); if (!base || map_end <= base || kEyeClientOffset + n > map_end - base) return {};
  const uint8_t* code = reinterpret_cast<const uint8_t*>(base + kEyeClientOffset);
  if (std::memcmp(code, expected, n) != 0) return {};
  return {reinterpret_cast<uintptr_t>(code), true};
}
}
