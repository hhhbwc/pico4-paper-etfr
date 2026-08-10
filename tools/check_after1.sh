#!/system/bin/sh
echo "=== restore后 confidence ==="
getprop sys.pxr.trackingservice.alg_confidence
echo "=== foveation level ==="
getprop persist.pvr.foveation.level
echo "=== algstate ==="
getprop sys.pxr.trackingservice.algstate
echo "=== 相关日志 ==="
su -c "logcat -d -t 150 2>/dev/null | grep -iE 'foveat|eyetrack|gaze|algstate' | tail -15"
echo "done"
