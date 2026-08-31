# USB capture milestone

The bridge dynamically discovers Paper USB devices by scanning sysfs and matching VID `0425`. It resolves the current bus node, CDC control/data interfaces, and bulk-IN/bulk-OUT endpoints at runtime, so it does not rely on a fixed `/dev/bus/usb` node or endpoint.

The observed Paper eye devices are `0425:0002` (left) and `0425:0003` (right). The active devices emit a continuous raw JPEG stream. The bridge also retains the incremental `JPG0` parser for Paper APK variants that use framed records.

All USB capture is explicit and bounded. `--dual`, `--diagnose-eyes`, `--dual-record`, `--target-record`, `--dual-live`, and `--wake-stream` accept only `1..30` seconds. A capture session claims the CDC interfaces, configures 115200 8N1 and DTR, optionally writes the statically recovered startup command `WAKE,L=50,F=40\n` to the discovered bulk-OUT endpoint, reads bulk-IN data, then releases every claimed interface and closes its USB node before returning.

`Capture()` now fails for CDC setup errors, device disconnects, non-timeout bulk errors, callback failures, and requested stop. Both-eye commands fail if either eye fails. `--dual-live` does not publish a final healthy heartbeat after failed capture, and `--target-record` rolls back rows appended for a failed target.

The daemon installs SIGINT/SIGTERM handlers. A signal only requests a stop; normal code exits the bounded read loop, joins both workers, closes files, releases USB ownership, and returns exit code `130`. A non-blocking `flock` lock prevents simultaneous daemon invocations from competing for the same USB interfaces or shared sample file; a competing invocation returns exit code `7`.

The Magisk service script creates a mode-`0700` runtime directory but deliberately does not start a persistent USB reader. On the tested headset, the calibration APK and manual commands must use Magisk `su -mm` so the daemon runs in the global USB mount namespace; ordinary `su -c` can leave the USB node inaccessible. Until a user-facing control protocol exists, all USB ownership remains in explicit bounded commands.
