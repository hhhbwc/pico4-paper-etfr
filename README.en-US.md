# PICO 4 + Paper Eye Tracker

A research and development project for connecting the third-party **Paper Tracker** to the standard-edition **PICO 4 (A8110)**. The immediate goal is a standalone bridge that captures and processes Paper eye images on the headset itself. Native eye-tracking game compatibility and eye-tracked foveated rendering (ETFR) are later validation stages, not completed features.

> 中文: [README.md](README.md) · Русский: [README.ru-RU.md](README.ru-RU.md)

## Project Status

The maintainable implementation is in [`paper-pico-bridge/`](paper-pico-bridge/). It is a separate CMake-based native project with a device daemon, transport parsers, JPEG decoding, pupil detection, calibration, a versioned shared-memory ABI, tests, and a read-only Zygisk runtime probe.

The verified device-side pipeline is:

```text
Paper USB eye devices
→ CDC setup and bulk-IN reads
→ raw JPEG reassembly
→ JPEG decoding on Pico
→ pupil candidate detection
→ left/right timestamp pairing
→ calibrated gaze sample publication
```

The current status is deliberately split into two modes:

- `--dual` is a diagnostic mode. It captures both eyes, decodes frames, runs the pupil detector, and reports statistics. It does not publish a valid gaze sample.
- `--dual-live <seconds> <calibration-file>` is the explicit live mode. It requires a valid binary calibration file, pairs observations within a 50 ms window, and publishes only fresh fused samples.
- `--target-record <id> <x> <y> <seconds> [csv]` appends labeled dual-eye observations for one currently displayed calibration target. A Pico-side display UI is not implemented yet.

Every USB capture command is bounded to 1 through 30 seconds and protected by an exclusive daemon lock. A competing invocation fails instead of competing for Paper USB. If either eye capture fails, live mode does not publish a completion heartbeat and target recording rolls back the rows from that target. The live mode is intentionally conservative: a missing, truncated, CSV, non-finite, or otherwise invalid calibration file is rejected, and only then does the daemon write a healthy heartbeat so a consumer cannot mistake an uncalibrated result for usable gaze data.

## What Has Been Verified

### Paper hardware and USB transport

Paper exposes three USB devices:

- Face: `0425:0001`
- Left eye: `0425:0002`
- Right eye: `0425:0003`

The bridge discovers the USB bus number, device address, CDC control interface, CDC data interface, and bulk-IN endpoint dynamically. It does not depend on a fixed `/dev/bus/usb` node or a fixed endpoint. The observed CDC configuration is 115200 baud, 8 data bits, 1 stop bit, with DTR asserted.

The current eye devices produce a continuous JPEG byte stream beginning with `FF D8` and ending with `FF D9`. Some Paper APK versions use framed `JPG0`/`UVC0` records; the bridge keeps a bounded incremental parser for that format as well as a raw-JPEG parser. Both parsers handle USB transfer boundaries and resynchronize after malformed input.

The daemon has been run on the headset with both eye devices at the same time. It can claim the control and data interfaces, configure the serial connection, read the stream, decode the JPEG, run the pupil detector, and release the interfaces when capture ends. It does not require a PC or a network gaze-coordinate service for this path.

### Device-side vision

The current vision backend is a small, portable baseline detector:

1. Decode the JPEG into a grayscale image.
2. Estimate image brightness and choose an adaptive darkness threshold.
3. Find connected components using four-neighbor connectivity.
4. Filter candidates by area, radius, aspect ratio, and confidence.
5. Return the strongest pupil candidate with pixel coordinates and confidence.

This is a functional baseline for integration testing, not a claim of parity with the Paper desktop model. The interface is isolated so a more accurate model can replace it later without changing the USB or shared-sample contracts.

### Calibration and shared samples

The current estimator fits an independent two-dimensional affine mapping for each eye. It maps pupil pixel coordinates to a normalized `[0,1]` gaze plane, clamps the result to that range, and produces per-eye and fused validity/confidence flags.

A valid persisted calibration file is exactly 216 bytes and contains:

```text
9 left-eye input points
9 right-eye input points
9 target points
```

Every coordinate must be a finite float, and both fits must be numerically valid. The timestamped CSV produced by `--dual-record` is observation data only: it has no target labels and is not a calibration file.

The shared `GazeSample` ABI contains a magic value, version, structure size, sequence number, monotonic timestamp, validity flags, left/right/fused vectors, confidence values, and source state. It is published with a sequence lock. A reader rejects samples that are malformed, being written, uncalibrated, unhealthy, stale, or non-finite.

The shared file used by the current prototype is:

```text
/data/local/tmp/paper-pico-bridge.sample
```

This is an internal prototype path, not a public network protocol.

## Architecture Findings

### Paper is an image source

Paper supplies eye images. In the original desktop workflow, gaze is computed from those images on the PC. The standalone bridge moves the capture, decoding, baseline vision, calibration, and sample publication onto Pico.

### The native Tobii image-input route is not the current plan

Reverse engineering showed that the active Pico Tobii algorithm consumes proprietary Tobii data and does not accept an ordinary Paper JPEG as a drop-in image source. Therefore, “feed Paper JPEGs into the native Tobii algorithm” is not treated as the implementation path.

The practical architecture is:

```text
Paper images
→ Pico-local vision and calibration
→ versioned shared GazeSample
→ verified OpenXR Runtime consumer compatibility layer
```

### Runtime consumer location

The current firmware contains the C++ method:

```text
pvr::TrackingClient::GetEyeTrackingData(long, int, pxr_eyepose*)
```

The bridge has verified the symbol shape, the current library signature, and the AArch64 argument convention. Its Zygisk component currently performs only:

- target-process identification;
- dynamic library-map lookup;
- known-build prologue validation;
- shared-sample freshness probing.

The active inline hook is disabled. Unknown runtime builds, invalid output layouts, and invalid or stale samples fail closed and preserve the original Pico behavior.

## Commands

The daemon supports these development commands:

```text
--enumerate
```

Dynamically discovers Paper devices and interfaces. It does not claim or reset them.

```text
--self-test
```

Creates or updates a test shared sample. It does not read USB devices.

```text
--capture <pid> <seconds>
```

Captures one selected Paper device and prints parser, JPEG, and pupil-detection diagnostics.

```text
--dual <seconds>
```

Captures both eye devices concurrently in diagnostic mode. It does not publish valid gaze.

```text
--dual-record <seconds> [csv]
```

Captures both eyes and writes timestamped pupil observations:

```text
timestamp_ns,eye,frame,x,y,radius,confidence,valid
```

```text
--dual-live <seconds> <calibration-file>
```

Loads the strict binary calibration, pairs recent left/right observations, and publishes only calibrated fused samples. If calibration loading fails, the command exits with code `4` and writes only a healthy heartbeat.

The default service mode is intentionally an idle safety skeleton. It does not automatically claim USB devices.

## Safety and Recovery Boundaries

The project is designed to preserve native Pico behavior whenever a prerequisite is not proven:

- no system-partition writes;
- no replacement of Pico system libraries;
- no fixed process IDs or fixed ASLR addresses;
- no automatic USB reset in the default service path;
- no active OpenXR inline patch in the packaged module;
- no gaze publication from stale, invalid, uncalibrated, or non-finite samples;
- original-function fallback in the isolated dispatch shim;
- strict runtime-build and prologue checks;
- bounded frame buffers and bounded capture durations.

The isolated ARM64 trampoline experiment includes original-byte storage, patch verification, instruction-cache flushing, a process-local patch mutex, sibling-thread stop/resume during the write window, and a restore path. It remains a development artifact. Generic PC-relative relocation, complete `pxr_eyepose` semantics, replacement lifetime, unload coordination, and real Runtime stability have not been proven.

## What Is Not Complete

The following items are still required before this can be described as a usable native-game bridge:

1. A user-facing nine-point calibration flow on Pico that produces a real binary calibration file.
2. Accuracy, jitter, latency, and dropout measurements with real calibration.
3. A verified mapping from the normalized Paper gaze plane to Pico's expected 3D gaze coordinate space.
4. A complete definition of `pxr_eyepose`, including field semantics, validity, timing, and coordinate conventions.
5. A replacement implementation that preserves all original fields and safely falls back to the original method.
6. A short, reversible Runtime compatibility test followed by long-duration stability testing.
7. Confirmation that a native eye-tracking game actually consumes the resulting data.
8. ETFR validation after the preceding steps; ETFR is not currently enabled or claimed to work.

## Build and Deployment

The bridge is built with CMake, Ninja, and an Android NDK targeting `arm64-v8a` and API 29. Set `ANDROID_NDK_HOME` before running `paper-pico-bridge/tools/build.ps1`:

```powershell
$env:ANDROID_NDK_HOME = 'C:\path\to\android-ndk'
./paper-pico-bridge/tools/build.ps1
```

The source tree excludes build directories, generated libraries, APKs, device captures, calibration recordings, and packaged ZIP files. The default Magisk skeleton contains the daemon and the read-only probe library; it is not a finished gaze-injection product.

## License

MIT
