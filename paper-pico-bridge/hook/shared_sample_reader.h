#pragma once
#include "shared_sample.h"
namespace paper_bridge {
class SharedSampleReader {
 public:
  ~SharedSampleReader();
  bool Open(const char* path);
  void Close();
  bool Read(uint64_t now_ns, uint64_t max_age_ns, GazeSample* out) const;
 private:
  int fd_ = -1; const SharedSample* shared_ = nullptr;
};
}
