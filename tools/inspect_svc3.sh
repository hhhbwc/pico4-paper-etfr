#!/system/bin/sh
echo "=== pxreyetrackingservice loaded eye/track/alg libs ==="
su -c "cat /proc/9614/maps | grep -oE '/[a-zA-Z0-9_./+-]+\.so' | sort -u | grep -iE 'eye|track|alg|phoenix|camera|pxr|pvr|client|tobii|standard'" 2>&1
echo ""
echo "=== all of them (full) ==="
su -c "cat /proc/9614/maps | grep -oE '/[a-zA-Z0-9_./+-]+\.so' | sort -u" 2>&1 | grep -iE "eye|track|alg|phoenix|camera"
echo ""
echo "=== exe path ==="
su -c "ls -l /proc/9614/exe" 2>&1
echo "done"
