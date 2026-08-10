#!/system/bin/sh
echo "=== pxreyetrackingservice (9614) full loaded libs ==="
cat /proc/9614/maps 2>/dev/null | grep -oE "/[a-zA-Z0-9_./+-]+\.so(\.1)?" | sort -u
echo ""
echo "=== is libpxreyeyetrackingservice loaded? (it links the impl) ==="
grep -E "libpxreyetrackingservice|pxreyetrack" /proc/9614/maps 2>/dev/null
echo ""
echo "=== all .so in /proc/9614/maps (exe region too) ==="
cat /proc/9614/maps 2>/dev/null | grep -E "r-xp" | awk '{print $6}' | sort -u | grep -E "\.so|pxrey"
echo "done"
