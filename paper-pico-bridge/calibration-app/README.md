# Pico calibration app

This is a small Java Activity for the PICO 4 2D virtual-display path. It draws a high-contrast 3x3 sequence and invokes the already-built root tools one target at a time:

```text
/data/local/tmp/paper_bridge_daemon --target-record <id> <x> <y> 3 /data/local/tmp/paper-pico-calibration-labeled.csv
/data/local/tmp/calibrate_csv /data/local/tmp/paper-pico-calibration-labeled.csv /data/local/tmp/paper-pico-calibration.bin
```

The APK does not contain USB, JPEG, pupil, calibration-fit, or OpenXR code. Before launching it, stage both ARM64 executables at those paths and connect the Paper eye devices. The app uses `su -c`, so a rooted device and root authorization are required.

The normalized target contract is fixed at 0.10, 0.50, and 0.90 on each axis. Each target has a 2.1-second visual countdown followed by a 3-second bounded capture. A nonzero daemon result leaves the current target available for retry. Back cancels the process and removes the incomplete CSV and artifact. After all nine targets, the app runs the strict artifact generator and accepts completion only when the root-side output is exactly 216 bytes.

The manifest starts with PICO metadata `pvr.2dtovr.mode=6` and `pico.vr.position=far`. On the tested A8110 firmware the Activity rendered on virtual Display 8 at 1602x902. The actual near/far presentation and whether the view is comfortable for calibration must be confirmed per firmware. This APK does not overlay VRChat or enable the OpenXR runtime hook.
