#!/system/bin/sh
echo "=== openxr 眼追 API 相关符号(完整) ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "GetEyeTracking|SetFoveation|EyeTracking|Foveat|GetEyePose|EyeGaze" | head -40
echo ""
echo "=== PxrEyeTrackingData 结构相关(找头文件线索) ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "gazeX|gazeY|gazeZ|gazeNormal|pupilDilation|gazeTarget|eyeOpen|PxrEyeTracking" | head
echo ""
echo "=== 哪个app用openxr眼追API ==="
for d in /system/priv-app /system/app /product/priv-app /product/app; do
  ls $d 2>/dev/null | while read ap; do
    so=$(find $d/$ap -name "*.so" 2>/dev/null | head -1)
    if [ -n "$so" ]; then
      H=$(strings "$so" 2>/dev/null | grep -icE "Pxr_GetEyeTracking|Pxr_SetFoveation")
      [ "$H" -gt "0" ] && echo "$ap : refs=$H"
    fi
  done
done
echo "scan done"
