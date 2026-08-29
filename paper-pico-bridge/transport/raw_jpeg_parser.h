#pragma once
#include <cstdint>
#include <vector>
namespace paper_bridge {
class RawJpegParser {
 public:
  explicit RawJpegParser(size_t max_frame = 262144) : max_frame_(max_frame) {}
  void Feed(const uint8_t* data, size_t size, std::vector<std::vector<uint8_t>>* out);
  void Reset();
 private:
  size_t max_frame_; std::vector<uint8_t> buffer_; bool in_frame_=false;
};
}
