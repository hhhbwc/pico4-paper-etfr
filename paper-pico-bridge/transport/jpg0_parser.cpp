#include "jpg0_parser.h"
#include <algorithm>
#include <cstring>

namespace paper_bridge {
static uint16_t U16(const uint8_t* p) { return uint16_t(p[0] | (p[1] << 8)); }
static uint32_t U32(const uint8_t* p) { return uint32_t(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)); }
Jpg0Parser::Jpg0Parser(size_t n) : max_buffer_(n) {}
void Jpg0Parser::Reset() { buffer_.clear(); }
void Jpg0Parser::Feed(const uint8_t* d, size_t n, std::vector<JpegFrame>* out) {
  if (!d || !n || !out) return;
  buffer_.insert(buffer_.end(), d, d + n);
  while (true) {
    size_t pos = 0;
    while (pos + 4 <= buffer_.size() &&
           std::memcmp(buffer_.data() + pos, "JPG0", 4) != 0 &&
           std::memcmp(buffer_.data() + pos, "UVC0", 4) != 0) ++pos;
    if (pos) buffer_.erase(buffer_.begin(), buffer_.begin() + pos);
    if (buffer_.size() < 8) break;
    if (std::memcmp(buffer_.data(), "UVC0", 4) == 0) {
      size_t total = size_t(buffer_[4]) + 8;
      if (total > max_buffer_) { buffer_.erase(buffer_.begin()); continue; }
      if (buffer_.size() < total) break;
      buffer_.erase(buffer_.begin(), buffer_.begin() + total); continue;
    }
    if (buffer_.size() < 12) break;
    uint16_t header = U16(buffer_.data() + 6); uint32_t payload = U32(buffer_.data() + 8);
    if (header < 12 || header >= 65 || payload == 0 || payload > 32768) {
      buffer_.erase(buffer_.begin()); continue;
    }
    size_t total = size_t(header) + payload;
    if (total > max_buffer_) { buffer_.erase(buffer_.begin()); continue; }
    if (buffer_.size() < total) break;
    const uint8_t* jpg = buffer_.data() + header;
    size_t end = payload;
    for (size_t i = payload; i >= 2; --i) if (jpg[i-2] == 0xff && jpg[i-1] == 0xd9) { end = i; break; }
    if (end >= 2 && jpg[0] == 0xff && jpg[1] == 0xd8 && jpg[end-2] == 0xff && jpg[end-1] == 0xd9)
      out->push_back({U16(buffer_.data() + 4), std::vector<uint8_t>(jpg, jpg + end)});
    buffer_.erase(buffer_.begin(), buffer_.begin() + total);
  }
  if (buffer_.size() > max_buffer_) buffer_.erase(buffer_.begin(), buffer_.end() - 16);
}
}
