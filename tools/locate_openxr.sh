#!/system/bin/sh
echo "=== locate libopenxr_api.so and libeyetrackingclient.pxr.so ==="
find /system /vendor /product -name "libopenxr_api.so" 2>/dev/null
find /system /vendor /product -name "libeyetrackingclient.pxr.so" 2>/dev/null
echo ""
echo "=== which process maps contain Pxr_GetEyeTrackingData consumers ==="
echo "=== openxr_runtime full .so list ==="
su -c "cat /proc/2009/maps | grep -oE '/[a-zA-Z0-9_./+-]+\.so' | sort -u | grep -iE 'openxr|eye|track|fove|pxr'" 2>&1
echo ""
echo "=== vrshell full .so list ==="
su -c "cat /proc/2456/maps | grep -oE '/[a-zA-Z0-9_./+-]+\.so' | sort -u | grep -iE 'openxr|eye|track|fove|pxr'" 2>&1
echo "done"
