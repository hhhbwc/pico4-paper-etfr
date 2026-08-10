#!/system/bin/sh
echo "=== pxreyetrackingservice 进程 ==="
ps -A 2>/dev/null | grep -iE "eyetrack|pxrey"
echo "=== 注册的 binder services ==="
service list 2>/dev/null | grep -iE "eye|track|pvr|pxr"
echo "=== 服务当前日志(是否有算法在跑) ==="
logcat -d 2>/dev/null | grep -iE "eyetrack|pxrey|pupil|gaze" | tail -15
