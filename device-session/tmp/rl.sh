#!/system/bin/sh
echo "=== libopenxr_api.so 的 Pxr 导出符号 ==="
readelf -sW /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "Pxr_GetEyeTracking|Pxr_SetFoveation|Pxr_StartEye" | head
echo "=== 用nm(若有) ==="
nm -D /system/lib64/libopenxr_api.so 2>/dev/null | grep -iE "Pxr_GetEyeTracking|Pxr_SetFoveation" | head
echo "=== 哪个进程加载了 libopenxr_api / libeyetrackingclient ==="
for pid in $(ps -A 2>/dev/null | awk '{print $2}'); do
  if grep -qa "libopenxr_api\|libeyetrackingclient" /proc/$pid/maps 2>/dev/null; then
    echo "pid=$pid $(cat /proc/$pid/cmdline 2>/dev/null | tr '\0' ' ') | openxr/eyetrack"
  fi
done
echo done
