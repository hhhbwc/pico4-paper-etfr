#!/system/bin/sh
# Switch algorithm Tobii->Standard: hook getAlgType to return 1 (Standard).
# Keep hasEyeCamera hook too. Observe camera/image pipeline activation.
R=/data/local/tmp/patch_ret1
GAT=0x7480342c8c
echo "=== 1. safe round-trip getAlgType patch ==="
su -c "$R apply 9614 $GAT 0 str" 2>&1
echo "--- alive? ---"; ps -A 2>/dev/null | grep -c " 9614 "
su -c "$R restore 9614 $GAT" 2>&1
echo "--- alive? ---"; ps -A 2>/dev/null | grep -c " 9614 "
echo ""
echo "=== 2. REAL: hook getAlgType->1 (Standard) + keep hasEyeCamera=true ==="
su -c "$R apply 9614 $GAT 0 str" 2>&1
echo "--- hasEyeCamera still patched? ---"
su -c "/data/local/tmp/ptrace_rw 9614 read 0x7480336cdc" 2>&1
echo ""
echo "=== 3. trigger: try to start algorithm via binder / observe ==="
echo "--- wait 3s for any reaction ---"
sleep 3
echo "--- algstate/conf ---"
getprop sys.pxr.trackingservice.algstate
getprop sys.pxr.trackingservice.alg_confidence
echo "--- recent eye/camera/alg logs ---"
su -c "logcat -d -t 60 2>/dev/null | grep -iE '9614|EyeTracking|CameraManager|openCamera|StartAlg|StandardAlg|Tobii|subscribe' | tail -15"
echo "--- alive? ---"
ps -A 2>/dev/null | grep -c " 9614 "
echo "done (hooks held: getAlgType=1, hasEyeCamera=true)"
