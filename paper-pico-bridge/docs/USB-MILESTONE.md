# USB capture milestone

The daemon now has a read-only Paper device discovery path. It scans `/dev/bus/usb`, maps candidate nodes through sysfs, and reports VID `0425` devices without resetting, selecting, or claiming any interface.

The modern Paper devices use PIDs `0500` and `0600` for the eye cameras. Their stream parser is implemented separately in `transport/jpg0_parser.*` and accepts `JPG0` records across arbitrary USB transfer boundaries.

Capture is intentionally not enabled yet. The current `Capture()` method only verifies that a device node can be opened, then closes it. Before enabling claims, validate on the target firmware:

1. Stop `com.bridge.papertracker` and confirm its USB sessions are closed.
2. Inspect the active descriptors and identify CDC control/data interfaces and bulk-IN endpoint dynamically.
3. Issue CDC line coding (`115200 8N1`) and DTR only after ownership is confirmed.
4. Capture a bounded sample and verify `JPG0` plus JPEG SOI/EOI.
5. Add reconnect and release handling before making the daemon persistent.

No device-side validation of this revision was possible when the ADB endpoint became unreachable. No system partition or Paper APK changes were made.
