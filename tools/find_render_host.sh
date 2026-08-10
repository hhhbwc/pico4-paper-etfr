#!/system/bin/sh
echo "=== 哪个进程加载了 eyetracking client / openxr ==="
for p in 2009 2265 2259 2456; do
  echo "--- pid $p ($(su -c "cat /proc/$p/cmdline 2>/dev/null | tr '\0' ' '")) ---"
  su -c "cat /proc/$p/maps 2>/dev/null | grep -oE '/[a-zA-Z0-9_./+-]+\.so' | sort -u | grep -iE 'eyetracking|openxr|foveat|pxrplugin|unity'" 2>&1 | head -12
done
echo "=== mrservice 是否消费 gaze (查日志) ==="
su -c "logcat -d 2>/dev/null | grep -iE 'mrservice|foveat|gaze|eyetrack' | grep -iE '2265|Foveation|gaze|eye' | tail -15"
echo "done"
