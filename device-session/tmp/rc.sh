#!/system/bin/sh
echo "=== 找 vrshell 渲染核心 ==="
find /system/priv-app /system/app /product/priv-app -iname "*vrshell*" -o -iname "*nativeshell*" 2>/dev/null | head
echo "=== 合成器/渲染库 是否引用眼追 ==="
for lib in /system/lib64/libpxrvrshell*.so /system/lib64/libxrshell.so /system/lib64/libcomposition*.so /vendor/lib64/*compositor*.so; do
  if [ -f "$lib" ]; then
    HIT=$(strings "$lib" 2>/dev/null | grep -icE "eyetrack|pupil|gaze|foveat|TrackingService|GetTrackingData")
    echo "$lib : eye/fovea refs=$HIT"
  fi
done
echo "=== openxr api 是否引用眼追 ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "gaze|eye|foveat|pupil|tracking" | head
