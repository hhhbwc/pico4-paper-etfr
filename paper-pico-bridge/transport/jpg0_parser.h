#pragma once
#include <cstdint>
#include <vector>

namespace paper_bridge {
struct JpegFrame { uint16_t sequence; std::vector<uint8_t> jpeg; };
class Jpg0Parser {
 public:
  explicit Jpg0Parser(size_t max_buffer = 131072);
  void Feed(const uint8_t* data, size_t size, std::vector<JpegFrame>* out);
  void Reset();
 private:
  size_t max_buffer_; std::vector<uint8_t> buffer_;
};
}
