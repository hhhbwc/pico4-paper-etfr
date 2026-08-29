#pragma once
#include <cstdint>
#include <vector>
namespace paper_bridge {
struct GrayImage { int width=0, height=0; std::vector<uint8_t> pixels; };
// Decoder backend boundary. JPEG support will be backed by a vetted embedded decoder.
bool DecodeJpegToGray(const uint8_t* jpeg, size_t size, GrayImage* output);
}
