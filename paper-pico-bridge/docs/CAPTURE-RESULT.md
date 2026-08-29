# Live capture result

Validated on Pico A8110 with Paper devices connected through USB Host:

- `0425:0001` face, `/dev/bus/usb/001/005`
- `0425:0002` left eye, `/dev/bus/usb/001/003`
- `0425:0003` right eye, `/dev/bus/usb/001/004`
- CDC control interface `0`, data interface `1`, bulk-IN `0x82`
- CDC setup: `115200 8N1`, DTR asserted
- Root daemon can claim/release a device without reset
- Right-eye capture: 48 complete JPEG frames in 2 seconds (~24 Hz), 7.0–7.5 KB/frame
- Captured JPEG decodes as `320x240 RGB`
- Raw stream is direct JPEG (`FF D8 ... FF D9`) for this firmware/device, not JPG0

The command used was explicitly bounded (`--capture 3 2`) and PaperTracker was stopped after a short initialization pass. The daemon released both interfaces at exit. Continuous daemon ownership is not enabled yet.
