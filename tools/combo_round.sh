#!/system/bin/sh
# Combined: FFR props enabled + gaze injection on openxr_runtime. Observe effect.
A=/data/local/tmp/inject_gaze
GED=0x7c6791e584   # openxr_runtime GetEyeTrackingData
echo "=== current FFR props ==="
getprop persist.pvr.config.enable_ffr
getprop persist.pvr.foveation.level
echo ""
echo "=== inject gaze on openxr_runtime, hold 6s ==="
su -c "$A 2009 apply $GED" 2>&1 | grep -E "PATCHED|verify"
echo "--- DURING ---"
getprop persist.pvr.foveation.level
getprop sys.pxr.trackingservice.alg_confidence
echo "--- openxr/render logs ---"
su -c "logcat -d -t 60 2>/dev/null | grep -iE '2009|foveat|Foveation|gaze|EyeTrack' | tail -12"
echo "--- alive ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo "--- hold 6s, watch level live ---"
for i in 1 2 3; do sleep 2; echo "t+${i}0s: foveation.level=$(getprop persist.pvr.foveation.level) conf=$(getprop sys.pxr.trackingservice.alg_confidence)"; done
echo "=== restore ==="
su -c "$A 2009 restore $GED" 2>&1 | grep -E "RESTORED|verify"
echo "--- after ---"
getprop persist.pvr.foveation.level
echo "combo round done"
