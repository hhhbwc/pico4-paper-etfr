#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_PNG
#define STBI_NO_GIF
#define STBI_NO_BMP
#define STBI_NO_TGA
#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image.h"
#include "jpeg_decoder.h"
#include <cstdlib>
namespace paper_bridge {
bool DecodeJpegToGray(const uint8_t* jpeg, size_t size, GrayImage* out) {
  if (!jpeg || !out || size == 0 || size > 262144) return false;
  int w=0,h=0,channels=0;
  unsigned char* pixels=stbi_load_from_memory(jpeg, static_cast<int>(size), &w, &h, &channels, 1);
  if (!pixels || w <= 0 || h <= 0) { if (pixels) stbi_image_free(pixels); return false; }
  out->width=w; out->height=h; out->pixels.assign(pixels,pixels+size_t(w)*h); stbi_image_free(pixels); return true;
}
}
