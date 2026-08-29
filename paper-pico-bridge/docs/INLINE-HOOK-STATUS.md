# Inline hook status

An isolated AArch64 trampoline experiment now builds, but it is not enabled in the Zygisk module. The controller still returns false unconditionally.

Implemented in the isolated experiment:

- exact 16-byte prologue match against the known runtime build;
- conservative relocation gate allowing only the observed SP-relative stack setup/save instructions;
- 16-byte trampoline with an AArch64 absolute literal jump, so it does not depend on a 26-bit branch range;
- process-local patch mutex;
- best-effort SIGSTOP/SIGCONT of sibling threads during the write window;
- original-byte and installed-patch copies, with patch-content verification before restore;
- instruction-cache flush and restoration of RX page permissions;
- rollback if permission restoration fails.

This is still not a production hook. The sibling-thread stop mechanism is only a bounded experiment and is not sufficient to prove runtime-wide quiescence. Generic ARM64 PC-relative relocation, replacement implementation, unload-time coordination, and crash-safe recovery remain unverified. The demangled method ABI is confirmed as:

```text
pvr::TrackingClient::GetEyeTrackingData(long, int, pxr_eyepose*)
// x0=this, x1=long, w2=int, x3=output pointer, x0=return value
```

The observed output offsets (`+0x48` validity and `+0x50..+0x58` gaze XYZ) are not a complete `pxr_eyepose` definition. Therefore the runtime probe, shared-sample freshness gate, output bounds/finite checks, and original-function fallback remain the only packaged behavior. No active Runtime patch or native-game gaze validation has been performed.


A standalone ABI dispatch shim is now covered by the core test target. It models the verified entry signature and only applies a fused sample when `ReadFreshSample` and `ApplyFusedGaze` both succeed; otherwise it calls the supplied original function and preserves its return value. This shim is not wired into the Zygisk lifecycle.
