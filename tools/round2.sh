#!/system/bin/sh
# ROUND 2: apply gaze (-0.5, 0.2, 0) to mrservice, READ during 5s hold, auto-restore
A=/data/local/tmp/inject_gaze
ADR=0x7c5c920584
echo "=== ROUND 2: apply gaze (-0.5,0.2,0) ==="
su -c "$A 2265 apply $ADR" 2>&1
echo "--- DURING PATCH: confidence/algstate/foveation ---"
getprop sys.pxr.trackingservice.alg_confidence
getprop sys.pxr.trackingservice.algstate
getprop persist.pvr.foveation.level
echo "--- DURING PATCH: recent foveation/gaze logs ---"
su -c "logcat -d -t 30 2>/dev/null | grep -iE 'foveat|gaze|eyetrack|confidence' | tail -6"
echo "--- mrservice alive? ---"
ps -A 2>/dev/null | grep -c " 2265 "
echo "--- hold 5s ---"
sleep 5
echo "=== ROUND 2: restore ==="
su -c "$A 2265 restore $ADR" 2>&1
echo "round2 done"
