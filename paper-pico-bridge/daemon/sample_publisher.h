#pragma once
#include "shared_sample.h"
namespace paper_bridge {
class SamplePublisher {
 public:
  SamplePublisher();
  ~SamplePublisher();
  bool Open(const char* path);
  void Close();
  SharedSample* shared() { return shared_; }
 private:
  int fd_ = -1; void* map_ = nullptr; SharedSample* shared_ = nullptr;
};
}
