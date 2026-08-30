# Paper Pico Bridge

Standalone Pico 4 A8110 bridge under active development.

Current milestone: host-testable JPG0/raw-JPEG parsers, versioned gaze shared-memory ABI, a strict nine-point calibration artifact generator (`calibrate_csv`), and explicit device-side capture commands. `--dual` remains diagnostic-only; `--dual-live <seconds> <calibration-file>` refuses to publish gaze without a valid persisted calibration. The project does not patch OpenXR.

All USB commands are bounded to `1..30` seconds and use a daemon lock. `--dual-record` writes unlabeled observations. `--target-record <id> <x> <y> <seconds> [csv]` appends a single labeled target's dual-eye observations, intended to be called once per target ID `0..8` by a future Pico calibration UI or another trusted display/controller. If either eye capture fails, the command returns failure and rolls back its appended rows.

The Magisk service only creates a mode-`0700` runtime directory; it does not start a persistent USB reader. Each explicit command claims USB only for its own bounded session and releases CDC interfaces and file descriptors before exit. SIGINT/SIGTERM request orderly shutdown, and a competing daemon invocation returns exit code `7` rather than competing for Paper USB ownership.

Build host tests with CMake. Android builds use the pinned NDK r27d and API 29. The future runtime hook must fail closed on unknown `libeyetrackingclient.pxr.so` builds and fall back to Pico's original eye-tracking function for stale or invalid samples.
