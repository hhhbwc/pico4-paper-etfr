#!/system/bin/sh
echo "=== Pxr_GetEyeTrackingData 相关字符串(结构字段) ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "gaze|eyeTracking|PxrEyeTracking|gazeRay|gazeNormal|eyePose|right|left|timeStamp|isDataValid|status|PxrEye" | head -40
echo ""
echo "=== openxr 头/扩展名 ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "XR_|xr_|EyeTracking|Foveat" | grep -iE "track|fovea|eye" | head -20
