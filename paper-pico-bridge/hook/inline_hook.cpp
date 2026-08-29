#include "inline_hook.h"
#include <sys/mman.h>
#include <sys/syscall.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <mutex>

namespace paper_bridge {
namespace {
static std::mutex g_patch_mutex;
constexpr size_t kPatchSize = 16;
constexpr uint32_t kNop = 0xd503201f;

static void flush(void* p, size_t n) {
  __builtin___clear_cache(static_cast<char*>(p), static_cast<char*>(p) + n);
}
static void* page_of(uintptr_t a) {
  return reinterpret_cast<void*>(a & ~(uintptr_t(getpagesize()) - 1));
}
static uint32_t insn(const uint8_t* p) {
  uint32_t v; std::memcpy(&v, p, sizeof(v)); return v;
}
static void put32(uint8_t* p, uint32_t v) { std::memcpy(p, &v, sizeof(v)); }

// The current verified prologue contains only SP-relative stack setup/save ops.
static bool relocatable_prologue(const uint8_t* p, size_t n) {
  if (!p || n != kPatchSize) return false;
  for (size_t i = 0; i < n; i += 4) {
    const uint32_t v = insn(p + i);
    const bool sub_sp = (v & 0xffc003ffu) == 0xd10003ffu;
    const bool str_sp = (v & 0xffc003e0u) == 0xf90003e0u;
    const bool stp_sp = (v & 0xffc003e0u) == 0xa90003e0u;
    if (!sub_sp && !str_sp && !stp_sp) return false;
  }
  return true;
}

// ldr x16, #8; br x16; <absolute address> (16 bytes total).
static void abs_jump(uint8_t* p, uintptr_t to) {
  put32(p + 0, 0x58000050u);
  put32(p + 4, 0xd61f0200u);
  uint64_t addr = static_cast<uint64_t>(to);
  std::memcpy(p + 8, &addr, sizeof(addr));
}

static bool stop_other_threads(std::vector<pid_t>* stopped) {
  if (!stopped) return false;
  const pid_t self = static_cast<pid_t>(syscall(SYS_gettid));
  DIR* dir = opendir("/proc/self/task");
  if (!dir) return false;
  while (dirent* e = readdir(dir)) {
    char* end = nullptr; long id = std::strtol(e->d_name, &end, 10);
    if (!end || *end || id <= 0 || id == self) continue;
    const pid_t tid = static_cast<pid_t>(id);
    if (syscall(SYS_tgkill, getpid(), tid, SIGSTOP) == 0) stopped->push_back(tid);
  }
  closedir(dir);
  // SIGSTOP is synchronous enough for this bounded patch window; avoid waiting
  // on /proc state because a thread may exit between enumeration and signal.
  usleep(1000);
  return true;
}
static void resume_threads(const std::vector<pid_t>& stopped) {
  for (pid_t tid : stopped) syscall(SYS_tgkill, getpid(), tid, SIGCONT);
}
static bool make_patchable(void* page, int prot) { return mprotect(page, getpagesize(), prot) == 0; }
}

bool InstallInlineHook(InlineHook* h, uintptr_t target, uintptr_t replacement,
                       const uint8_t* expected, size_t n) {
#if !defined(__aarch64__)
  (void)h; (void)target; (void)replacement; (void)expected; (void)n; return false;
#else
  std::lock_guard<std::mutex> guard(g_patch_mutex);
  if (!h || h->installed || !target || !replacement || n != kPatchSize ||
      !expected || std::memcmp(reinterpret_cast<void*>(target), expected, n) != 0 ||
      !relocatable_prologue(expected, n)) return false;
  void* tramp = mmap(nullptr, 32, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (tramp == MAP_FAILED) return false;
  std::memcpy(h->original, reinterpret_cast<void*>(target), kPatchSize);
  std::memcpy(tramp, h->original, kPatchSize);
  abs_jump(static_cast<uint8_t*>(tramp) + kPatchSize, target + kPatchSize);
  flush(tramp, 32);

  std::vector<pid_t> stopped;
  void* page = page_of(target);
  if (!stop_other_threads(&stopped) || !make_patchable(page, PROT_READ | PROT_WRITE | PROT_EXEC)) {
    resume_threads(stopped); munmap(tramp, 32); return false;
  }
  uint8_t patch[kPatchSize]{};
  abs_jump(patch, replacement);
  std::memcpy(reinterpret_cast<void*>(target), patch, kPatchSize);
  std::memcpy(h->installed_patch, patch, kPatchSize);
  flush(reinterpret_cast<void*>(target), kPatchSize);
  bool ok = make_patchable(page, PROT_READ | PROT_EXEC);
  resume_threads(stopped);
  if (!ok) {
    make_patchable(page, PROT_READ | PROT_WRITE | PROT_EXEC);
    std::memcpy(reinterpret_cast<void*>(target), h->original, kPatchSize);
    flush(reinterpret_cast<void*>(target), kPatchSize);
    make_patchable(page, PROT_READ | PROT_EXEC);
    munmap(tramp, 32); return false;
  }
  h->target = target; h->replacement = replacement;
  h->trampoline = reinterpret_cast<uintptr_t>(tramp);
  h->patch_size = kPatchSize; h->installed = true;
  return true;
#endif
}

bool RemoveInlineHook(InlineHook* h) {
#if !defined(__aarch64__)
  (void)h; return false;
#else
  std::lock_guard<std::mutex> guard(g_patch_mutex);
  if (!h || !h->installed || h->patch_size != kPatchSize) return false;
  if (std::memcmp(reinterpret_cast<void*>(h->target), h->installed_patch, kPatchSize) != 0) return false;
  std::vector<pid_t> stopped;
  void* page = page_of(h->target);
  if (!stop_other_threads(&stopped) || !make_patchable(page, PROT_READ | PROT_WRITE | PROT_EXEC)) {
    resume_threads(stopped); return false;
  }
  std::memcpy(reinterpret_cast<void*>(h->target), h->original, kPatchSize);
  flush(reinterpret_cast<void*>(h->target), kPatchSize);
  bool ok = make_patchable(page, PROT_READ | PROT_EXEC);
  resume_threads(stopped);
  if (!ok) return false;
  munmap(reinterpret_cast<void*>(h->trampoline), 32);
  h->installed = false; h->patch_size = 0; h->trampoline = 0;
  return true;
#endif
}
}
