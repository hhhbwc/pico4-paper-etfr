# Paper Pico Bridge

Standalone Pico 4 A8110 bridge under active development.

Current milestone: host-testable JPG0/raw-JPEG parsers, versioned gaze shared-memory ABI, a strict nine-point calibration artifact generator (`calibrate_csv`), and explicit device-side capture commands. `--dual` remains diagnostic-only; `--diagnose-eyes` adds read-only image statistics and transform diagnostics; `--wake-stream <seconds>` performs the statically recovered `WAKE,L=50,F=40\n` startup write and verifies both JPEG streams without launching PaperTracker; `--dual-live <seconds> <calibration-file>` refuses to publish gaze without a valid persisted calibration. The project does not patch OpenXR.

All USB commands are bounded to `1..30` seconds and use a daemon lock. `--dual-record` writes unlabeled observations. `--target-record <id> <x> <y> <seconds> [csv]` appends a single labeled target's dual-eye observations for one target ID `0..8`. The companion Java calibration Activity provides one-tap automatic nine-point orchestration on PICO's 2D virtual-display path, but that path still needs real headset visibility and input-routing validation. If either eye capture fails, the command returns failure and rolls back its appended rows.

The Magisk service only creates a mode-`0700` runtime directory; it does not start a persistent USB reader. Each explicit command claims USB only for its own bounded session and releases CDC interfaces and file descriptors before exit. SIGINT/SIGTERM request orderly shutdown, and a competing daemon invocation returns exit code `7` rather than competing for Paper USB ownership.

Build host tests with CMake. Android builds use the pinned NDK r27d and API 29. The future runtime hook must fail closed on unknown `libeyetrackingclient.pxr.so` builds and fall back to Pico's original eye-tracking function for stale or invalid samples.
