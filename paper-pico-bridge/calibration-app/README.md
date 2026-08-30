# Pico calibration app

This is a small Java Activity for the PICO 4 2D virtual-display path. It draws a high-contrast 3x3 sequence and invokes the already-built root tools one target at a time:

```text
/data/local/tmp/paper_bridge_daemon --target-record <id> <x> <y> 3 /data/local/tmp/paper-pico-calibration-labeled.csv
/data/local/tmp/calibrate_csv /data/local/tmp/paper-pico-calibration-labeled.csv /data/local/tmp/paper-pico-calibration.bin
```

The APK does not contain USB, JPEG, pupil, calibration-fit, or OpenXR code. Before launching it, stage both ARM64 executables at those paths and connect the Paper eye devices. The app uses `su -c`, so a rooted device and root authorization are required.

The normalized target contract is fixed at 0.10, 0.50, and 0.90 on each axis. One tap first performs a two-second daemon-only dual-eye JPEG readiness check, then starts the complete automatic nine-point run with a 750 ms settle period and three-second bounded capture per target. The APK never launches, foregrounds, or stops Paper Tracker. A failed point is retried automatically up to three attempts and never advances without both eyes producing valid samples. After three failures the app stops at that target for a manual retry; it never writes an incomplete calibration artifact. Back cancels the active root process and removes only the incomplete CSV. If Android recreates the Activity during a session, the current target is shown again and must be reacquired. After all nine targets, the app runs the strict artifact generator and accepts completion only when the root-side output is exactly 216 bytes; independent validation with separate user samples is still a later runtime-validation step.

The readiness check intentionally does not fabricate an unknown Paper USB startup command. On a cold device state without a confirmed stream-start protocol it fails while leaving this calibration Activity visible; that state requires protocol recovery before unattended calibration can be considered usable.

The manifest starts with PICO metadata `pvr.2dtovr.mode=6` and `pico.vr.position=far`. On the tested A8110 firmware the Activity rendered on virtual Display 8 at 1602x902. The actual near/far presentation and whether the view is comfortable for calibration must be confirmed per firmware. This APK does not overlay VRChat or enable the OpenXR runtime hook.
