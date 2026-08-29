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

The CSV is observation data only. It is not a calibration file: it has no target labels and cannot be passed to `--dual-live`. A valid binary calibration is exactly 216 bytes: nine left-eye input points, nine right-eye input points, and nine target points, with finite float coordinates. `CalibrationStore::Load` rejects any other length, truncated content, non-finite value, or non-invertible fit. No gaze validity or OpenXR injection is enabled until a user-facing nine-point calibration sequence has collected known target coordinates and fitted both eye mappings. A live run must point to the binary file produced by `CalibrationStore::Save`; if that file is missing or invalid, `--dual-live` exits with code 4 and publishes only a healthy heartbeat.
