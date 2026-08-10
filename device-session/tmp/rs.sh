#!/system/bin/sh
echo "=== 找 vrshell 的 so ==="
find /system/priv-app/VRShell2 -name "*.so" 2>/dev/null
echo "=== vrshell libs 里的 eye tracking 结构字段 ==="
for so in $(find /system/priv-app/VRShell2 -name "*.so" 2>/dev/null); do
  H=$(strings "$so" 2>/dev/null | grep -icE "gazeRay|PxrEyeTracking|gazePosition|eyeData|rightEyePose")
  [ "$H" -gt "0" ] && echo "--- $so ---" && strings "$so" 2>/dev/null | grep -iE "PxrEyeTracking|gazeRay|gazePosition|eyePose|eyeTrackingData|timeStamp" | head
done
echo "=== 系统其它含PxrEyeTracking字段的so ==="
for so in /system/lib64/*.so /vendor/lib64/*.so; do
  if strings "$so" 2>/dev/null | grep -q "PxrEyeTrackingData"; then echo "FOUND: $so"; fi
done 2>/dev/null
echo done
