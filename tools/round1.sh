#!/system/bin/sh
# ROUND 1: apply gaze patch to mrservice, hold 5s, auto-restore
A=/data/local/tmp/inject_gaze
ADR=0x7c5c920584   # mrservice GetEyeTrackingData (base 7c5c90d000 + 0x13584)
echo "=== ROUND 1: apply gaze (0.2,0,0) ==="
su -c "$A 2265 apply $ADR" 2>&1
echo "patched, holding 5s..."
# capture foveation/confidence right after patch
su -c "logcat -d -t 50 2>/dev/null | grep -iE 'foveat|eyetrack|gaze|alg_confidence' | tail -8"
getprop sys.pxr.trackingservice.alg_confidence 2>/dev/null
echo "--- hold ---"
sleep 5
echo "=== ROUND 1: restore ==="
su -c "$A 2265 restore $ADR" 2>&1
echo "round1 done"
