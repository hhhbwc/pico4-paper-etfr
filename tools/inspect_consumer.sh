#!/system/bin/sh
# inspect openxr_runtime + vrshell: what eye/foveation libs they load
echo "=== openxr_runtime (2009) eye/fovea libs ==="
cat /proc/2009/maps 2>/dev/null | grep -oE "/[a-zA-Z0-9_./+-]+\.so(\.1)?" | sort -u | grep -iE "eye|fove|track|pxr|pvr|openxr|alg|phoenix|client"
echo ""
echo "=== vrshell (2456) eye/fovea libs ==="
cat /proc/2456/maps 2>/dev/null | grep -oE "/[a-zA-Z0-9_./+-]+\.so(\.1)?" | sort -u | grep -iE "eye|fove|track|pxr|pvr|openxr|client"
echo ""
echo "=== openxr_runtime loaded libopenxr / libeyetracking? ==="
grep -E "openxr|eyetrack|fove" /proc/2009/maps 2>/dev/null | head -20
echo ""
echo "=== vrshell loaded libopenxr / libeyetracking? ==="
grep -E "openxr|eyetrack|fove" /proc/2456/maps 2>/dev/null | head -20
echo "done"
