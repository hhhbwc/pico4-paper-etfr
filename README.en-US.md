# PICO 4 + Paper Eye Tracker → System-Level Eye-Tracked Foveated Rendering

Reverse-engineering project to drive **Eye-Tracked Foveated Rendering (ETFR)** on **PICO 4 (A8110)** using the third-party **Paper Tracker** eye tracker.

> 中文: [README.md](README.md) · Русский: [README.ru-RU.md](README.ru-RU.md)

## Goal

The PICO 4 **standard edition has no eye-tracking hardware**, but its system (same as the Pro edition) **fully retains the software chain for eye tracking + foveated rendering**. This project injects third-party eye-tracker (Paper) data to make the system believe it has native eye tracking, enabling ETFR (rendering follows the gaze to save GPU).

## Key Findings (confirmed by reverse engineering)

### 1. Paper transmits "images", not "coordinates"
- Sensors are **3 ESP32-S3 USB devices**: Left Eye (0425:0002) / Right Eye (0425:0003) / Face (0425:0001)
- PICO app (`com.bridge.papertracker`) reads them directly via **libusb** (no kernel FTDI driver)
- **PICO → PC sends only eye images (240×320 grayscale JPEG)**, UDP unicast `192.168.1.119:35506 → PC:45454`
- **Gaze coordinates are computed on the PC from the image** (that's Paper's "PC processes data")
- Frame protocol: `PT` header (5054 0101) + frame counter + chunk index + length + JPEG

### 2. PICO rendering layer has full ETFR API (chain is open)
Exported from `libopenxr_api.so`:
- `Pxr_GetEyeTrackingData` / `Pxr_GetEyeTrackingData1`
- `Pxr_StartEyeTracking` / `Pxr_StopEyeTracking`
- `Pxr_GetEyeTrackingSupported` / `Pxr_GetEyeTrackingState`
- `Pxr_SetFoveationLevel` / `Pxr_SetFoveationParams` / `Pxr_GetLayerFoveationImage`

**Standard edition shares the same system as Pro → the consumption chain exists, only the data source is missing.**

### 3. Eye-tracking service is complete but "idle"
`pxreyetrackingservice` (registered binder `pvr.IEyeTrackingService`):
```
The status of eye camera: Nonexistent   <- standard edition has no eye camera
ET algorithm is not started.
isSupportEyetracking = 0
```
The service is fully functional and binder-callable, just **doesn't start because it detects no hardware. `isSupportEyetracking` is a software check (h)ookable**.

### 4. Internal architecture
`libpxreyetrackingservice.so` / `libpxreyetracking.phoenix.so`:
```
CameraManager::openCamera/addImageListener → onFrameAvailable  <- camera image
AlgorithmBase::setResultsListener / getTrackingDataSharedMemory  <- algorithm
TrackingService::GetData / SetData / GetTrackingDataSharedMemory  <- data output
```

## Injection Path (identified, to be implemented)

**Plan (hijack the system, reuse native algorithm + rendering):**
1. **Hook `isSupportEyetracking` → return true** (master switch)
2. **Make CameraManager think there's a camera**: feed Paper images to the system
3. System algorithm runs → computes gaze → rendering consumes → **ETFR enabled**

**Or alternative: hook `Pxr_GetEyeTrackingData` / `SetData` to inject coordinates directly.**

## Tools (`tools/`)
- `usbcdc_read.c` — USB CDC reader (NDK build)
- `ptrace_probe.c` — ptrace injection probe (attach confirmed working)
- `list_9100.ps1` / `cap_osc.ps1` — PC-side OSC/network analysis

## TODO / Blockers
- [ ] ptrace inject `pxreyetrackingservice` (root system service, needs dedicated session)
- [ ] Hook `isSupportEyetracking` + inject data source
- [ ] Reverse system algorithm data structure (DataBufferParcelable gaze format)
- [ ] VR test whether rendering responds (foveation follows)

## License
MIT
