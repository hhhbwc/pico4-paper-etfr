#!/system/bin/sh
echo "=== dumpsys 眼追服务 ==="
dumpsys pxreyetrackingservice 2>/dev/null | head -40
echo "=== service call 尝试(查询支持) - 试多个code ==="
for code in 1 2 3 4 5 6 7 8; do
  echo "--- code=$code ---"
  service call pxreyetrackingservice $code 2>&1 | head -3
done
