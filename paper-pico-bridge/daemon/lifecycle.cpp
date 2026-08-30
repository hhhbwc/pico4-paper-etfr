#include "lifecycle.h"
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace paper_bridge {
namespace {
volatile sig_atomic_t g_stop = 0;
void stop_handler(int) { g_stop = 1; }
}

bool ParseBoundedSeconds(const char* text, int* seconds) {
  if (!text || !seconds || !*text) return false;
  char* end = nullptr;
  errno = 0;
  long value = std::strtol(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0' || value < 1 || value > 30) return false;
  *seconds = static_cast<int>(value);
  return true;
}

void InstallStopHandlers() {
  struct sigaction action{};
  action.sa_handler = stop_handler;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGTERM, &action, nullptr);
}

bool StopRequested() { return g_stop != 0; }
void RequestStop() { g_stop = 1; }

int AcquireDaemonLock(const char* path) {
  if (!path) return -1;
  int fd = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
  if (fd < 0 || flock(fd, LOCK_EX | LOCK_NB) != 0) {
    if (fd >= 0) close(fd);
    return -1;
  }
  return fd;
}

void ReleaseDaemonLock(int fd) {
  if (fd >= 0) {
    flock(fd, LOCK_UN);
    close(fd);
  }
}
}
