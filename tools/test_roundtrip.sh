#!/system/bin/sh
# Safety round-trip test on openxr_runtime(2009): apply -> verify -> restore
# GetEyeTrackingData runtime addr in 2009 = base 0x7c6790b000 + 0x13584 = 0x7c6791e584
A=/data/local/tmp/inject_gaze
echo "=== ROUND-TRIP TEST on openxr_runtime(2009) ==="
echo "--- 1. apply patch ---"
su -c "$A 2009 apply 0x7c6791e584" 2>&1
echo "--- 2. restore (immediately) ---"
su -c "$A 2009 restore 0x7c6791e584" 2>&1
echo "--- 3. confirm mrservice/openxr still alive after ---"
ps -A 2>/dev/null | grep -E " 2009 | 2265 " | awk '{print $1, $NF}'
echo "=== test done ==="
