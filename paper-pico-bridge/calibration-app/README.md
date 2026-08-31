# Pico calibration app

This is a small Java Activity for the PICO 4 2D virtual-display path. It draws a high-contrast 3x3 sequence and invokes the already-built root tools one target at a time:

```text
/data/local/tmp/paper_bridge_daemon --target-record <id> <x> <y> 3 /data/local/tmp/paper-pico-calibration-labeled.csv
/data/local/tmp/calibrate_csv /data/local/tmp/paper-pico-calibration-labeled.csv /data/local/tmp/paper-pico-calibration.bin
```

The APK does not contain USB, JPEG, pupil, calibration-fit, or OpenXR code. Before launching it, stage both ARM64 executables at those paths and connect the Paper eye devices. The app uses `su -mm -c` so the daemon inherits the global USB mount namespace; a rooted device and root authorization are required.

The normalized target contract is fixed at 0.10, 0.50, and 0.90 on each axis. One tap first performs a bounded daemon-native stream wake (`WAKE,L=50,F=40\n`, the startup default recovered from the Paper APK) and a three-second dual-eye JPEG verification, then starts the complete automatic nine-point run with a 750 ms settle period and three-second bounded capture per target. The APK never launches, foregrounds, or stops Paper Tracker. A failed point is retried automatically up to three attempts and never advances without both eyes producing valid samples. After three failures the app stops at that target for a manual retry; it never writes an incomplete calibration artifact. Back cancels the active root process and removes only the incomplete CSV. If Android recreates the Activity during a session, the current target is shown again and must be reacquired. After all nine targets, the app runs the strict artifact generator and accepts completion only when the root-side output is exactly 216 bytes; independent validation with separate user samples is still a later runtime-validation step.

The wake check uses only the statically recovered Paper startup write (`WAKE,L=50,F=40\n`) and fails closed on USB permission, claim, write, or JPEG verification errors while leaving this calibration Activity visible. On the tested A8110, launching the daemon through `su -mm -c` allowed both eye nodes to open and both streams to produce complete JPEG frames without PaperTracker. Ordinary `su -c` may remain in a mount namespace where the USB nodes cannot be opened.

The manifest starts with PICO metadata `pvr.2dtovr.mode=6` and `pico.vr.position=far`. The Activity is assigned to a PICO private virtual display at 1602x902, but a shell-launched smoke test left that display `OFF` with the PICO shell retaining focus. Real headset presentation, input routing, and near/far comfort therefore remain unverified. This APK does not overlay VRChat or enable the OpenXR runtime hook.
