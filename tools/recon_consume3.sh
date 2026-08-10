#!/system/bin/sh
echo "=== openxr_runtime apk/lib 位置 ==="
pm path com.pico.xr.openxr_runtime 2>/dev/null
APK=$(pm path com.pico.xr.openxr_runtime 2>/dev/null | grep base | sed 's/package://')
echo "APK=$APK"
echo "=== runtime 里的 foveation/eyetracking 消费 ==="
# check the runtime's native libs
for so in $(find /system/priv-app/com.pico.xr.openxr_runtime /system/app/com.pico.xr.openxr_runtime -name "*.so" 2>/dev/null); do
  H=$(strings "$so" 2>/dev/null | grep -icE "foveat|eyeTrack|gaze|GetFoveation")
  echo "$so refs=$H"
done
echo "=== 全局找 foveation 消费库 ==="
for so in $(find /vendor/lib64 /system/lib64 -name "*.so" 2>/dev/null | xargs grep -l "Foveat" 2>/dev/null); do
  echo "HAS Foveat: $so"
done
echo done
