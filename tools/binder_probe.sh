#!/system/bin/sh
echo "=== binder transaction codes of IpvEyeTrackingService ==="
# List all binder services matching eye/track
service list 2>/dev/null | grep -iE "eye|track"
echo ""
echo "=== try IEyeTrackingService transaction (SetData ~ a few codes) ==="
# pxreyetrackingservice registers as service 3: pvr.IEyeTrackingService
# binder interface IpvEyeTrackingService. Try calling with dummy int+parcel
for code in 1 2 3 4 5; do
  echo "--- code=$code (SetData attempt) ---"
  service call pxreyetrackingservice $code i32 0 2>&1 | head -5
done
echo ""
echo "=== algstate/confidence BEFORE ==="
getprop sys.pxr.trackingservice.alg_confidence
getprop sys.pxr.trackingservice.algstate
echo "done"
