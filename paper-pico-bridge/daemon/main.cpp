#include "shared_sample.h"
#include "paper_usb.h"
#include "sample_publisher.h"
#include "lifecycle.h"
#include "jpg0_parser.h"
#include "raw_jpeg_parser.h"
#include "jpeg_decoder.h"
#include "pupil_detector.h"
#include "gaze_estimator.h"
#include <chrono>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace paper_bridge;

namespace {
constexpr const char* kDaemonLockPath = "/data/local/tmp/paper-pico-bridge.lock";

uint64_t now_ns() {
  timespec t{};
  clock_gettime(CLOCK_MONOTONIC, &t);
  return uint64_t(t.tv_sec) * 1000000000ull + uint64_t(t.tv_nsec);
}

bool parse_target(const char* text, float* value) {
  if (!text || !value) return false;
  char* end = nullptr;
  errno = 0;
  float parsed = std::strtof(text, &end);
  if (errno != 0 || end == text || *end != '\0' || !std::isfinite(parsed)) return false;
  *value = parsed;
  return true;
}

void publish_heartbeat(SamplePublisher* publisher) {
  if (!publisher || !publisher->shared()) return;
  GazeSample heartbeat{};
  heartbeat.magic = kSampleMagic;
  heartbeat.version = kSampleVersion;
  heartbeat.size = sizeof(heartbeat);
  heartbeat.sequence = now_ns();
  heartbeat.monotonic_ns = heartbeat.sequence;
  heartbeat.flags = kDaemonHealthy;
  PublishSample(publisher->shared(), heartbeat);
}

struct CaptureResult { bool ok = false; size_t frames = 0; size_t valid = 0; size_t bytes = 0; std::string diagnostic; };

CaptureResult capture_eye(uint16_t pid, int seconds, const char* label,
                          const std::function<void(const PupilObservation&)>& on_observation) {
  PaperUsb usb;
  UsbDeviceInfo info;
  CaptureResult result;
  if (!usb.Find(pid, &info, &result.diagnostic)) return result;
  RawJpegParser raw;
  result.ok = usb.Capture(info, seconds * 1000, [&](const UsbDeviceInfo&, const uint8_t* data, size_t n) {
    result.bytes += n;
    std::vector<std::vector<uint8_t>> frames;
    raw.Feed(data, n, &frames);
    for (const auto& frame : frames) {
      GrayImage image;
      if (!DecodeJpegToGray(frame.data(), frame.size(), &image)) continue;
      PupilObservation observation = PupilDetector().Detect(image.pixels.data(), image.width, image.height);
      ++result.frames;
      if (observation.valid) ++result.valid;
      if (on_observation) on_observation(observation);
    }
  }, &result.diagnostic);
  std::cerr << label << " " << result.diagnostic << " frames=" << result.frames
            << " valid=" << result.valid << " bytes=" << result.bytes << "\n";
  return result;
}

int run_target_record(int argc, char** argv) {
  if (argc < 6 || argc > 7) return 2;
  char* end = nullptr;
  errno = 0;
  long target_id = std::strtol(argv[2], &end, 10);
  float target_x = 0, target_y = 0;
  int seconds = 0;
  if (errno != 0 || end == argv[2] || *end != '\0' || target_id < 0 || target_id > 8 ||
      !parse_target(argv[3], &target_x) || !parse_target(argv[4], &target_y) ||
      !ParseBoundedSeconds(argv[5], &seconds)) {
    std::cerr << "usage: --target-record <target_id 0..8> <target_x> <target_y> <seconds 1..30> [csv]\n";
    return 2;
  }
  const char* path = argc == 7 ? argv[6] : "/data/local/tmp/paper-pico-calibration-labeled.csv";
  FILE* csv = std::fopen(path, "ab+");
  if (!csv) { std::perror(path); return 5; }
  std::fseek(csv, 0, SEEK_END);
  const long start_offset = std::ftell(csv);
  if (start_offset == 0) std::fprintf(csv, "target_id,target_x,target_y,eye,x,y,confidence,valid,timestamp_ns,frame_index,radius\n");
  std::mutex output;
  auto worker = [&](uint16_t pid, const char* label, CaptureResult* result) {
    size_t frame = 0;
    *result = capture_eye(pid, seconds, label, [&](const PupilObservation& p) {
      std::lock_guard<std::mutex> lock(output);
      std::fprintf(csv, "%ld,%.6f,%.6f,%s,%.4f,%.4f,%.5f,%d,%llu,%zu,%.4f\n", target_id, target_x, target_y,
                   label, p.x, p.y, p.confidence, p.valid ? 1 : 0,
                   static_cast<unsigned long long>(now_ns()), ++frame, p.radius);
    });
  };
  CaptureResult left, right;
  std::thread left_thread(worker, 2, "left", &left), right_thread(worker, 3, "right", &right);
  left_thread.join();
  right_thread.join();
  std::fflush(csv);
  if (!left.ok || !right.ok || left.valid == 0 || right.valid == 0) {
    const int fd = fileno(csv);
    if (start_offset >= 0) ftruncate(fd, start_offset);
    std::fclose(csv);
    if (start_offset == 0) std::remove(path);
    std::cerr << "target capture failed; appended rows rolled back\n";
    return StopRequested() ? 130 : 6;
  }
  std::fclose(csv);
  return 0;
}

int run_dual_record(int argc, char** argv) {
  if (argc < 3 || argc > 4) return 2;
  int seconds = 0;
  if (!ParseBoundedSeconds(argv[2], &seconds)) { std::cerr << "usage: --dual-record <seconds 1..30> [csv]\n"; return 2; }
  const char* path = argc == 4 ? argv[3] : "/data/local/tmp/paper-pico-calibration.csv";
  FILE* csv = std::fopen(path, "wb");
  if (!csv) { std::perror(path); return 5; }
  std::fprintf(csv, "timestamp_ns,eye,frame,x,y,radius,confidence,valid\n");
  std::mutex output;
  auto worker = [&](uint16_t pid, const char* label, CaptureResult* result) {
    size_t frame = 0;
    *result = capture_eye(pid, seconds, label, [&](const PupilObservation& p) {
      std::lock_guard<std::mutex> lock(output);
      std::fprintf(csv, "%llu,%s,%zu,%.4f,%.4f,%.4f,%.5f,%d\n", static_cast<unsigned long long>(now_ns()),
                   label, ++frame, p.x, p.y, p.radius, p.confidence, p.valid ? 1 : 0);
    });
  };
  CaptureResult left, right;
  std::thread left_thread(worker, 2, "left", &left), right_thread(worker, 3, "right", &right);
  left_thread.join();
  right_thread.join();
  std::fclose(csv);
  if (!left.ok || !right.ok || left.frames == 0 || right.frames == 0)
    return StopRequested() ? 130 : 6;
  return 0;
}

int run_dual_live(int argc, char** argv) {
  if (argc != 4) { std::cerr << "usage: --dual-live <seconds 1..30> <calibration-file>\n"; return 2; }
  int seconds = 0;
  if (!ParseBoundedSeconds(argv[2], &seconds)) { std::cerr << "invalid live arguments\n"; return 2; }
  SamplePublisher publisher;
  if (!publisher.Open("/data/local/tmp/paper-pico-bridge.sample")) { std::cerr << "sample open failed\n"; return 3; }
  GazeEstimator estimator;
  if (!estimator.LoadCalibration(argv[3])) { publish_heartbeat(&publisher); std::cerr << "calibration unavailable; live mode refused\n"; return 4; }
  struct EyeState { PupilObservation observation{}; uint64_t timestamp = 0; bool seen = false; };
  EyeState eyes[2];
  std::mutex state_mutex;
  auto worker = [&](uint16_t pid, int index, const char* label, CaptureResult* result) {
    *result = capture_eye(pid, seconds, label, [&](const PupilObservation& p) {
      const uint64_t stamp = now_ns();
      std::lock_guard<std::mutex> lock(state_mutex);
      eyes[index] = {p, stamp, true};
      const EyeState other = eyes[index ^ 1];
      const uint64_t delta = stamp >= other.timestamp ? stamp - other.timestamp : other.timestamp - stamp;
      if (!p.valid || !other.seen || !other.observation.valid || delta > 50000000ull) return;
      GazeSample sample{};
      if (estimator.Estimate(eyes[0].observation, eyes[1].observation, stamp, &sample) && (sample.flags & kFusedValid))
        PublishSample(publisher.shared(), sample);
    });
  };
  CaptureResult left, right;
  std::thread left_thread(worker, 2, 0, "left", &left), right_thread(worker, 3, 1, "right", &right);
  left_thread.join();
  right_thread.join();
  if (!left.ok || !right.ok) { std::cerr << "live capture failed; no healthy completion published\n"; return StopRequested() ? 130 : 6; }
  publish_heartbeat(&publisher);
  return 0;
}

int run_dual(int argc, char** argv) {
  if (argc != 3) { std::cerr << "usage: --dual <seconds 1..30>\n"; return 2; }
  int seconds = 0;
  if (!ParseBoundedSeconds(argv[2], &seconds)) { std::cerr << "invalid dual arguments\n"; return 2; }
  auto worker = [&](uint16_t pid, const char* label, CaptureResult* result) {
    size_t shown = 0;
    *result = capture_eye(pid, seconds, label, [&](const PupilObservation& p) {
      if (shown++ < 3) std::cout << label << " pupil=" << p.valid << " center=" << p.x << "," << p.y
                                  << " confidence=" << p.confidence << "\n";
    });
  };
  CaptureResult left, right;
  std::thread left_thread(worker, 2, "left", &left), right_thread(worker, 3, "right", &right);
  left_thread.join();
  right_thread.join();
  if (!left.ok || !right.ok || left.frames == 0 || right.frames == 0) {
    std::cerr << "dual stream unavailable\n";
    return StopRequested() ? 130 : 6;
  }
  return 0;
}
}

int main(int argc, char** argv) {
  InstallStopHandlers();
  ScopedDaemonLock lock(kDaemonLockPath);
  if (!lock.acquired()) { std::cerr << "another paper bridge daemon is running\n"; return 7; }
  try {
    if (argc == 2 && std::string(argv[1]) == "--enumerate") {
      PaperUsb usb;
      std::string diagnostic;
      bool ok = usb.Enumerate(&diagnostic);
      std::cout << diagnostic << "\n";
      return ok ? 0 : 1;
    }
    if (argc > 1 && std::string(argv[1]) == "--target-record") return run_target_record(argc, argv);
    if (argc > 1 && std::string(argv[1]) == "--dual-record") return run_dual_record(argc, argv);
    if (argc > 1 && std::string(argv[1]) == "--dual-live") return run_dual_live(argc, argv);
    if (argc > 1 && std::string(argv[1]) == "--dual") return run_dual(argc, argv);
    if (argc == 2 && std::string(argv[1]) == "--service") {
      while (!StopRequested()) std::this_thread::sleep_for(std::chrono::milliseconds(200));
      return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--self-test") {
      SamplePublisher publisher;
      if (!publisher.Open("/data/local/tmp/paper-pico-bridge.sample")) return 3;
      GazeSample sample{};
      sample.magic = kSampleMagic;
      sample.version = kSampleVersion;
      sample.size = sizeof(sample);
      sample.monotonic_ns = now_ns();
      sample.flags = kCalibrated | kDaemonHealthy | kFusedValid;
      sample.fused_xyz[2] = 1.0f;
      PublishSample(publisher.shared(), sample);
      std::cout << "sample published\n";
      return 0;
    }
    std::cerr << "usage: --service | --enumerate | --dual <seconds> | --dual-record <seconds> [csv] | --target-record <id> <x> <y> <seconds> [csv] | --dual-live <seconds> <calibration-file> | --self-test\n";
    return 2;
  } catch (...) {
    std::cerr << "invalid daemon arguments\n";
    return 2;
  }
}
