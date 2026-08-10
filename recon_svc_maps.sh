#!/system/bin/sh
PID=$(pidof pxreyetrackingservice)
echo "pxreyetrackingservice pid=$PID"
echo "=== 它加载的眼追相关库 ==="
cat /proc/$PID/maps 2>/dev/null | grep -oE "/[a-zA-Z0-9_./-]+\.so" | sort -u | grep -iE "eye|track|phoenix|camera|pxr|alg" | head -30
echo "=== libpxreyetracking.phoenix.so 是否加载基址 ==="
grep "libpxreyetracking" /proc/$PID/maps 2>/dev/null | head
