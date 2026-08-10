#!/system/bin/sh
# Round-trip test then real hasEyeCamera hook on pxreyetrackingservice(9614)
A=/data/local/tmp/patch_func
HAS=0x7480336cdc   # TrackingService::hasEyeCamera
echo "=== safe round-trip ==="
su -c "$A apply 9614 $HAS" 2>&1
echo "--- alive? ---"
ps -A 2>/dev/null | grep -c " 9614 "
su -c "$A restore 9614 $HAS" 2>&1
echo "--- alive after? ---"
ps -A 2>/dev/null | grep -c " 9614 "
echo "=== round-trip OK, now REAL hook ==="
su -c "$A apply 9614 $HAS" 2>&1
echo "=== hasEyeCamera patched (ret true). Watch service react ==="
echo "--- immediate logs ---"
su -c "logcat -d -t 60 2>/dev/null | grep -iE '9614|EyeTracking|hasEye|openCamera|startAlg|isSupportEye' | tail -15"
getprop sys.pxr.trackingservice.algstate
getprop sys.pxr.trackingservice.alg_confidence
echo "--- service alive? ---"
ps -A 2>/dev/null | grep -c " 9614 "
echo "done (hook held)"
