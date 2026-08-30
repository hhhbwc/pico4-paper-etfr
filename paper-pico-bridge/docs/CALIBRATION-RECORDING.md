# Calibration recording milestone

Added the `--dual-record <seconds> [csv]` daemon mode. It captures both eyes concurrently, decodes JPEG on Pico, runs the pupil detector, and writes timestamped observations:

```text
timestamp_ns,eye,frame,x,y,radius,confidence,valid
```

Warm run on the connected A8110:

- 5 seconds
- 114 frames per eye (~22.8 FPS)
- 228 observation rows
- 195 rows marked valid by the detector
- Left/right streams were captured concurrently and released cleanly

The CSV is observation data only. It is not a calibration file: it has no target labels and cannot be passed to `--dual-live`. For a display/controller that can show one target at a time, use the bounded device-side command:

```text
--target-record <target_id> <target_x> <target_y> <seconds> [csv]
```

It appends both eyes to a labeled CSV, including a monotonic timestamp:

```text
target_id,target_x,target_y,eye,x,y,confidence,valid,timestamp_ns
```

The command does not display targets or fit calibration; it only binds the currently displayed target to concurrent Paper observations. The optional `calibration-app/` Android APK provides one-tap automatic orchestration on PICO's 2D virtual display: it first runs a daemon-only two-second dual-eye JPEG readiness check, then advances from target ID `0` through `8` and retries a failed target up to three times without skipping it. It never launches, foregrounds, or stops the Paper Tracker application, so it cannot obstruct or replace the calibration view. The daemon returns failure when either eye has no complete JPEG frame; no unknown outbound USB command is sent to force a cold stream. It requires both ARM64 tools to be staged at `/data/local/tmp/` and root authorization. Recreate a point after an interrupted Activity session; only completed, validated output should be used. For externally labeled data, the `calibrate_csv <labeled.csv> <output.bin>` tool accepts this format:

```text
target_id,target_x,target_y,eye,x,y,confidence,valid[,timestamp_ns]
```

It requires all nine target IDs for both eyes, ignores invalid or low-confidence rows, requires at least three accepted samples per eye and target, rejects excessive per-coordinate median absolute deviation (MAD), uses a trimmed per-coordinate aggregate, fits independent two-dimensional projective homographies, checks left-to-right and top-to-bottom direction, performs a round-trip load check, and writes a valid binary calibration. A valid calibration is exactly 216 bytes: nine left-eye input points, nine right-eye input points, and nine target points, with finite float coordinates. This is Pico raw control-point format v1; it is not the proprietary PC `config.ini` or homography-parameter format. `CalibrationStore::Load` rejects any other length, truncated content, non-finite value, or non-invertible fit. No gaze validity or OpenXR injection is enabled until a user-facing nine-point calibration sequence has collected known target coordinates and fitted both eye mappings. A live run must point to the binary file produced by `CalibrationStore::Save`; if that file is missing or invalid, `--dual-live` exits with code 4 and publishes only a healthy heartbeat.
