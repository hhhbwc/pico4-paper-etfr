#!/system/bin/sh
echo "=== pxreyetrackingservice 数据发布相关符号 ==="
strings /system/lib64/libpxreyetrackingservice.so 2>/dev/null | grep -iE "SharedMemory|Parcelable|Notify|Broadcast|callback|Listener|Result|gaze|pupil|GetData|setData|DataBuffer|getResults" | head -30
echo ""
echo "=== TrackingService 完整方法(读导出) ==="
# find exported symbols via strings containing method patterns
strings /system/lib64/libpxreyetrackingservice.so 2>/dev/null | grep -iE "^_ZN3pvr15TrackingService" | head -30
