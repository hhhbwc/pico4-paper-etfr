#!/system/bin/sh
echo "=== 搜 supportEyetracking / isSupportEyetracking 相关 prop ==="
getprop | grep -iE "eye|track|support" | head -20
echo "=== 硬件检测 prop ==="
getprop | grep -iE "camera|hw\.|board|persist.*eye" | head
echo "=== ro.product / hardware ==="
getprop ro.product.name 2>/dev/null
getprop ro.hardware 2>/dev/null
getprop ro.boot.hardware 2>/dev/null
echo "=== persist 里 eye 相关 ==="
getprop persist | grep -iE "eye|track|et|ft" | head
