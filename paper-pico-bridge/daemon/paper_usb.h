#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <atomic>
namespace paper_bridge {
struct UsbDeviceInfo { int bus=0, address=0; uint16_t vid=0, pid=0; std::string path; int control_if=-1, data_if=-1, bulk_in=-1; };
using FrameCallback = std::function<void(const UsbDeviceInfo&, const uint8_t*, size_t)>;
class PaperUsb {
 public:
  bool Enumerate(std::string* diagnostic) const;
  bool Find(uint16_t pid, UsbDeviceInfo* out, std::string* diagnostic) const;
  bool Capture(const UsbDeviceInfo&, int duration_ms, const FrameCallback&, std::string* diagnostic);
  void Stop();
 private:
  std::atomic<bool> stop_requested_{false};
};
}
