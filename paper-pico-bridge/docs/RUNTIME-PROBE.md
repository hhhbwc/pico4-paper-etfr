# Runtime probe result

The current `libeyetrackingclient.pxr.so` exports the C++ method:

```text
_ZN3pvr14TrackingClient18GetEyeTrackingDataEliP11pxr_eyepose
```

at relative offset `0x13584`. `libopenxr_api.so` exports wrapper functions but does not import this C++ symbol, so the public Zygisk PLT hook API cannot directly replace the method.

The Zygisk module now performs a read-only post-specialization probe:

- resolves the library base from `/proc/self/maps`;
- derives base + `0x13584`;
- checks the known 16-byte prologue;
- reads the shared sample freshness;
- logs the result;
- leaves Runtime code untouched.

The current-build disassembly also confirms that `x3` is retained as the output object pointer and that the original method copies a substantially larger structure than three floats. Observed accesses include writes at `+0x00..+0x20`, `+0x30..+0x48`, `+0x50..+0x68`, `+0x70..+0x88`, and `+0xa0..+0xa7`; later reads include `+0x48`, `+0x50`, `+0x54`, `+0x58`, `+0x64`, `+0x68`, `+0x70`, `+0x74`, `+0x7c`, `+0x80`, and `+0x84`. These are access offsets, not field semantics. In particular, `+0x48` cannot safely be treated as a standalone validity byte without observing the complete producer/consumer contract.

An active inline trampoline is intentionally not enabled. It requires preserving the ARM64 prologue, relocating any PC-relative instructions, handling the C++ method ABI, and proving the output object layout on this exact build. Until those are independently tested, the safe fallback is preferable to risking OpenXR stability.
