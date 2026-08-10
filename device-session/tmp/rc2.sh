#!/system/bin/sh
echo "=== openxr_api 所有 eye/fovea/gaze/FFR 符号 ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "eye|fovea|gaze|pupil|FFR|Foveat|gazeTarget|EyeGaze" | head -30
echo ""
echo "=== 是否有 Pxr EyeGaze / Foveation API ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "Pxr.*Eye|Pxr.*Gaze|Pxr.*Fovea|EyeTrack|Gaze" | head -20
echo ""
echo "=== vrshell 渲染合成(查找含 eye/fovea 的 so) ==="
find /system/priv-app/VRShell2 -name "*.so" 2>/dev/null | while read f; do
  HIT=$(strings "$f" 2>/dev/null | grep -icE "eyetrack|gaze|foveat|pupil")
  [ "$HIT" -gt "0" ] && echo "$f : refs=$HIT"
done
echo "scan done"
