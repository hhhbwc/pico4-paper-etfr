# Zygisk milestone

The published Zygisk module API header is vendored as `hook/zygisk.hpp` from the official zygisk-module-sample source. The module now:

- loads through the public `ModuleBase` API;
- reads the specialized app's JNI process name;
- closes itself for all non-target processes;
- recognizes only `com.pico.xr.openxr_runtime`;
- opens the shared sample file only after app specialization;
- verifies sample freshness through the shared ABI;
- logs the probe result and leaves the original Runtime untouched.

The packaged library is still a probe, not an active eye-data hook. The function patch is intentionally disabled until the exact in-process hook mechanism and output ABI are validated against the current runtime. This prevents an installable module from modifying unknown code paths.
