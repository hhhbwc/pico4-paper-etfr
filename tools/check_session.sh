#!/system/bin/sh
echo "=== 2265/2259 还在吗 ==="
ps -A 2>/dev/null | grep -E " 2265 | 2259 "
echo "=== 它们是谁 ==="
for p in 2265 2259; do
  echo -n "pid $p: "
  su -c "cat /proc/$p/cmdline 2>/dev/null | tr '\0' ' '"
  echo ""
done
echo "=== openxr/渲染相关进程 ==="
ps -A 2>/dev/null | grep -iE "openxr|vrshell|compositor|render|unity|game"
echo "=== 前台 activity ==="
dumpsys activity activities 2>/dev/null | grep -iE "mResumedActivity|topResumedActivity" | head -3
echo "=== 有没有正在渲染的 XR 会话(compositor 客户端) ==="
dumpsys SurfaceFlinger --list 2>/dev/null | head -30
echo "done"
