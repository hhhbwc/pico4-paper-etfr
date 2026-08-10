#!/system/bin/sh
# Correct target: openxr_runtime(2009), the real eye-tracking consumer.
# Get dynamic base, apply gaze patch, hold 5s, read state, restore.
A=/data/local/tmp/inject_gaze
BASE=$(su -c "cat /proc/2009/maps | grep libeyetrackingclient.pxr.so | head -1 | cut -d'-' -f1")
echo "openxr_runtime eyetrackingclient base = $BASE"
# full hex address without truncation
GED=$(printf '0x%x' $((0x$BASE + 0x13584)))
echo "GetEyeTrackingData runtime = $GED"
echo ""
echo "=== APPLY gaze patch on openxr_runtime(2009) ==="
su -c "$A 2009 apply $GED" 2>&1
echo "--- DURING: confidence/algstate/foveation ---"
getprop sys.pxr.trackingservice.alg_confidence
getprop sys.pxr.trackingservice.algstate
getprop persist.pvr.foveation.level
echo "--- openxr_runtime alive? ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo "--- hold 5s ---"
sleep 5
echo "=== RESTORE ==="
su -c "$A 2009 restore $GED" 2>&1
echo "round-openxr done"
