# Vision milestone

Validated on Pico A8110 with both eye devices connected:

- Concurrent root capture from PIDs `0002` (left) and `0003` (right)
- 3-second run: 70 frames per eye (~23.3 FPS)
- Bytes: left 583268, right 509655
- JPEG decode on-device: 320x240 RGB input to grayscale image
- Lightweight pupil detector produced valid observations on 69/70 left frames and 70/70 right frames
- Example centers vary with the observed eye image and include confidence values

The first initialization pass may be required after reconnecting hardware: launching PaperTracker briefly and stopping it causes both devices to begin streaming. The daemon then claims and releases both interfaces independently. This is temporary bring-up behavior; production startup must implement an explicit device wake/start command or a safe coordinator.

The current detector is a baseline candidate finder, not the final calibrated PaperTracker model. Calibration and gaze mapping remain required before any OpenXR injection.
