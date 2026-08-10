#!/system/bin/sh
# Preflight: verify mrservice(2265) session alive + read-only probe of injection target
# NO triggering yet. Just confirm we CAN read mrservice's GetEyeTrackingData code.
echo "=== mrservice session active? ==="
su -c "logcat -d 2>/dev/null | grep -iE '2265' | grep -iE 'XR_SESSION_STATE|mr render' | tail -3"
echo ""
# dynamic base
BASE=$(su -c "cat /proc/2265/maps | grep libeyetrackingclient.pxr.so | head -1 | cut -d'-' -f1")
echo "eyetrackingclient base = $BASE"
if [ -n "$BASE" ]; then
  # convert hex to decimal for arithmetic
  GED=$((0x$BASE + 0x13584))   # TrackingClient::GetEyeTrackingData
  GED2=$((0x$BASE + 0x1386c))  # GetEyeTrackingData_2_0
  echo "GetEyeTrackingData @ $GED"
  # read first instruction (word) to confirm readable
  su -c "/data/local/tmp/ptrace_rw 2265 read $GED" 2>&1
fi
echo "done (no trigger, read-only verify)"
