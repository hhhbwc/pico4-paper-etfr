# Gaze milestone

The project now has a gaze-estimation boundary between pupil observations and the shared ABI:

- Separate left/right calibration fits
- Coordinate clamping to normalized `[0,1]`
- Per-eye and fused confidence/validity flags
- Monotonic timestamp and sequence assignment
- Persistent calibration storage with atomic temporary-file rename
- Output adapter with strict object-size, offset, finite-value, calibration, and fused-valid checks

An explicit `--dual-live <seconds> <calibration-file>` daemon mode now connects the two validated pieces: it loads the persisted affine calibration, pairs left/right observations within a 50 ms window, and publishes only fused, calibrated samples to `/data/local/tmp/paper-pico-bridge.sample`. At capture end it publishes a healthy-only heartbeat so stale gaze is not accepted. Missing or invalid calibration refuses live mode with exit code 4 and writes only a healthy heartbeat.

The existing `--dual` mode remains diagnostic-only and does not publish gaze. The adapter is not installed into OpenXR. The runtime hook remains fail-closed until an official Zygisk API header and a verified firmware-specific prologue/output layout are available. Invalid or stale samples must continue to fall back to Pico's original function.
