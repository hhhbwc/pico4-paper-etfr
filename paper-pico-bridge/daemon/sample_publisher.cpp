#include "sample_publisher.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
namespace paper_bridge {
SamplePublisher::SamplePublisher() = default;
SamplePublisher::~SamplePublisher() { Close(); }
bool SamplePublisher::Open(const char* path) {
  if (!path) return false;
  fd_ = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
  if (fd_ < 0 || ftruncate(fd_, sizeof(SharedSample)) != 0) { Close(); return false; }
  map_ = mmap(nullptr, sizeof(SharedSample), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (map_ == MAP_FAILED) { map_ = nullptr; Close(); return false; }
  shared_ = static_cast<SharedSample*>(map_);
  InitializeShared(shared_); return true;
}
void SamplePublisher::Close() {
  if (map_) munmap(map_, sizeof(SharedSample));
  if (fd_ >= 0) close(fd_);
  map_ = nullptr; shared_ = nullptr; fd_ = -1;
}
}
