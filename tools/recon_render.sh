#!/system/bin/sh
echo "=== openxr runtime 位置 ==="
find /system /vendor /product -iname "*openxr*" 2>/dev/null | head
echo "=== 找渲染合成核心(vrshell/display/compositor) ==="
ls /system/lib64/ /vendor/lib64/ 2>/dev/null | grep -iE "openxr|compositor|display|render|frame" | head -20
echo "=== 眼追相关的渲染消费: vrshell 是否引用 eyetracking ==="
VRSHELL=$(ls /system/priv-app/com.pvr.vrshell/*.so /system/app/com.pvr.vrshell/*.so 2>/dev/null | head -1)
echo "vrshell so candidate: $VRSHELL"
strings /system/lib64/libpxreyetrackingservice.so 2>/dev/null | grep -iE "gaze|pupil|foveat|callback|listener|compositor|display" | head -20
