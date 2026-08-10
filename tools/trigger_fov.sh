#!/system/bin/sh
# TRIGGER: force FFR on (SetFoveationLevel 9) + gaze inject on openxr_runtime, hold ~8s
A=/data/local/tmp/inject_foveation
GED=0x7c6791e584
SF=0x7cd26dba74
echo "=== TRIGGER: apply foveation(9)+gaze ==="
su -c "$A apply 2009 $GED $SF" 2>&1 | grep -E "apply|verify"
echo "--- DURING: live state ---"
for i in 1 2 3; do
  sleep 2
  echo "t+${i}0s: foveation.level=$(getprop persist.pvr.foveation.level) conf=$(getprop sys.pxr.trackingservice.alg_confidence) enable_ffr=$(getprop persist.pvr.config.enable_ffr)"
done
echo "--- render/foveation logs ---"
su -c "logcat -d -t 80 2>/dev/null | grep -iE 'Foveation|foveat|2009.*[Ff]ovea|PxrPlugin' | tail -10"
echo "--- alive ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo "=== RESTORE ==="
su -c "$A restore 2009 $GED" 2>&1 | grep -E "restored"
echo "--- after ---"
getprop persist.pvr.foveation.level
echo "trigger done"
