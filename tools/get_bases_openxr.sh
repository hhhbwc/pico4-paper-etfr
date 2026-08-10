#!/system/bin/sh
echo "=== openxr_runtime(2009) libopenxr_api base ==="
su -c "cat /proc/2009/maps | grep libopenxr_api.so | head -1 | cut -d'-' -f1"
echo "=== openxr_runtime eyetrackingclient base ==="
su -c "cat /proc/2009/maps | grep libeyetrackingclient.pxr.so | head -1 | cut -d'-' -f1"
echo "=== 当前会话活跃? ==="
su -c "logcat -d -t 20 2>/dev/null | grep -iE '2009|APxrRuntime|PxrCompositor|foveat' | tail -6"
echo "=== 前台 ==="
dumpsys activity activities 2>/dev/null | grep -iE 'mResumedActivity' | head -2
echo "done"
