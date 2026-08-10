#!/system/bin/sh
# Safe round-trip test of inject_foveation on openxr_runtime(2009)
A=/data/local/tmp/inject_foveation
GED=0x7c6791e584     # GetEyeTrackingData
SF=0x7cd26dba74      # Pxr_SetFoveationLevel(9)
echo "=== round-trip: apply ==="
su -c "$A apply 2009 $GED $SF" 2>&1
echo "--- alive? ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo "--- restore immediately ---"
su -c "$A restore 2009 $GED" 2>&1
echo "--- alive after? ---"
ps -A 2>/dev/null | grep -c " 2009 "
echo "roundtrip done"
