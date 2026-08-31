# Vision milestone

Validated on Pico A8110 with both eye devices connected:

- Concurrent root capture from PIDs `0002` (left) and `0003` (right)
- Repeated 1–3 second runs: approximately 22–24 decoded JPEG frames per second per eye
- JPEG decode on-device: 320x240 RGB input to grayscale image
- The daemon-native wake path succeeds without PaperTracker in the foreground or background when launched with Magisk `su -mm`
- Wake diagnostics show CDC `115200 8N1`, DTR asserted, a 15-byte bulk-OUT write of `WAKE,L=50,F=40\n`, and complete JPEG frames from both eyes

The current detector is only a baseline candidate finder. Recent live frames consistently decode from both eyes, but the right-eye stream produced `0/N` baseline-valid candidates while the left eye produced variable valid counts. A diagnostic-only comparison found that grayscale inversion makes right-eye candidates pass the baseline shape gate, but this is not evidence that those candidates are pupils and is not used by calibration. The saved frames show substantial non-eye scene content, so a Paper-compatible ROI/preprocessing model and correct headset presentation still need to be established.

The calibration APK uses `su -mm -c` and never launches, foregrounds, or stops PaperTracker. Its Activity is placed in a PICO private virtual display, but that display was observed `OFF` with no content while the PICO shell retained focus; real headset visibility and input routing remain unverified.

Calibration, 3D head-pose/ray mapping, and gaze mapping remain required before any OpenXR injection. No visible PaperTracker launch, `RESTART`, guessed, random, or fuzzed command is used.
