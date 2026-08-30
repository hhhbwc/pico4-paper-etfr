#pragma once

namespace paper_bridge {

bool ParseBoundedSeconds(const char* text, int* seconds);
void InstallStopHandlers();
bool StopRequested();
void RequestStop();
int AcquireDaemonLock(const char* path);
void ReleaseDaemonLock(int fd);

class ScopedDaemonLock {
 public:
  explicit ScopedDaemonLock(const char* path) : fd_(AcquireDaemonLock(path)) {}
  ~ScopedDaemonLock() { ReleaseDaemonLock(fd_); }
  bool acquired() const { return fd_ >= 0; }
 private:
  int fd_;
};

}
