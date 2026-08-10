#!/system/bin/sh
echo "=== openxr runtime 进程 ==="
ps -A 2>/dev/null | grep -iE "openxr|xr_runtime|xrshell|vrshell"
echo "=== openxr 相关包 ==="
pm list packages 2>/dev/null | grep -iE "openxr|xr_runtime"
echo "=== libopenxr_api 里的 EyeTrackingSupported 实现来源 ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "EyeTrackingSupported|hasEyeCamera|eyeCamera|GetEyeTracking|eyetrack" | head
echo "=== openxr loader 指向的 runtime ==="
strings /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "/system/lib|libopenxr|runtime|\.so" | grep -iE "xr|load" | head
