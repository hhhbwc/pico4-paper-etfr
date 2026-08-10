#!/system/bin/sh
# Round-trip + trigger on openxr_runtime(2009), the REAL eye consumer.
# GetEyeTrackingData runtime = 0x7c6791e584 (base 0x7c6790b000 + 0x13584)
A=/data/local/tmp/inject_gaze
GED=0x7c6791e584
echo "=== openxr_runtime(2009) round-trip safety test ==="
su -c "$A 2009 apply $GED" 2>&1
su -c "$A 2009 restore $GED" 2>&1
echo "--- alive? ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo ""
echo "=== TRIGGER: apply, hold 5s, read live state ==="
su -c "$A 2009 apply $GED" 2>&1
echo "--- DURING PATCH ---"
getprop sys.pxr.trackingservice.alg_confidence
getprop sys.pxr.trackingservice.algstate
getprop persist.pvr.foveation.level
echo "--- recent openxr eye logs ---"
su -c "logcat -d -t 30 2>/dev/null | grep -iE '2009|EyeTracking|gaze|foveat' | tail -8"
echo "--- hold 5s ---"
sleep 5
echo "=== RESTORE ==="
su -c "$A 2009 restore $GED" 2>&1
echo "--- after restore ---"
getprop sys.pxr.trackingservice.alg_confidence
getprop persist.pvr.foveation.level
echo "--- alive after? ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo "openxr round done"
