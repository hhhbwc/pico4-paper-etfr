# Paper Pico Bridge

Standalone Pico 4 A8110 bridge under active development.

Current milestone: host-testable JPG0/raw-JPEG parsers, versioned gaze shared-memory ABI, a strict nine-point calibration artifact generator (`calibrate_csv`), and an explicit device-side `--dual-live <seconds> <calibration-file>` path. `--dual` remains diagnostic-only; `--dual-live` refuses to publish gaze without a valid persisted calibration. The daemon's default service mode does not reset or claim USB devices, and the project does not patch OpenXR.

Build host tests with CMake. Android builds use the pinned NDK r27d and API 29. The future runtime hook must fail closed on unknown `libeyetrackingclient.pxr.so` builds and fall back to Pico's original eye-tracking function for stale or invalid samples.
