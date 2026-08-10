#!/system/bin/sh
echo "=== mrservice(2265) libeyetrackingclient.pxr.so base ==="
su -c "cat /proc/2265/maps | grep libeyetrackingclient.pxr.so" 2>&1
echo ""
echo "=== mrservice(2265) libopenxr_api.so base ==="
su -c "cat /proc/2265/maps | grep libopenxr_api.so" 2>&1
echo ""
echo "=== mrservice(2265) libpxreyetrackingservice.so (服务端?) loaded? ==="
su -c "cat /proc/2265/maps | grep -iE 'eyetrackingservice|phoenix' " 2>&1
echo ""
echo "=== 确认 mrservice 会话状态 (当前 active?) ==="
su -c "logcat -d 2>/dev/null | grep -iE '2265' | grep -iE 'XR_SESSION_STATE|mr render' | tail -5"
echo "done"
