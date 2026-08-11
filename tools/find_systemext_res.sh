#!/system/bin/sh
echo "=== systemext 进程 ==="
ps -A 2>/dev/null | grep -iE "systemext"
echo ""
echo "=== systemext / 2D分辨率相关属性 ==="
getprop | grep -iE "ns_|app_window|2d|window_size|flat" | head -20
echo ""
echo "=== config db 里 2D/window/分辨率 相关 ==="
DB=/data/user_de/0/com.pvr.configuration/databases/config.db
su -c "sqlite3 $DB \"SELECT * FROM config WHERE key LIKE '%window%' OR key LIKE '%2d%' OR key LIKE '%res%' OR key LIKE '%screen%' OR key LIKE '%app%' LIMIT 60\" 2>&1" | head -40
echo ""
echo "=== systemext 里 NS_APP 分辨率相关 so/文件 ==="
su -c "find /system/priv-app/com.picovr.systemext -name '*.so' 2>/dev/null | head"
echo "--- systemext 是否有 native so 含 1602/902 ---"
for so in $(su -c "find /system/priv-app/com.picovr.systemext -name '*.so' 2>/dev/null"); do
  H=$(su -c "strings $so 2>/dev/null | grep -cE '1602|902|1600|900'")
  [ "$H" -gt 0 ] && echo "$so : hits=$H"
done
echo "done"
