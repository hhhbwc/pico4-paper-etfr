#pragma once
#include <cstddef>
#include <cstdint>
namespace paper_bridge {
constexpr uintptr_t kEyeClientOffset = 0x13584;
constexpr char kKnownEyeClientSha256[] = "7a7568f9501b5e383056e1bca68295813c3043b08e838a954bf1c188e6220cb7";
constexpr uint8_t kKnownGetEyeTrackingDataPrologue[16] = {
  0xff,0x83,0x02,0xd1,0xf9,0x2b,0x00,0xf9,
  0xf8,0x5f,0x06,0xa9,0xf6,0x57,0x07,0xa9
};
}
