#!/system/bin/sh
echo "=== enable_ffr 当前值 + 可写性测试 ==="
getprop persist.pvr.config.enable_ffr
echo "--- 尝试读取 config db 里的 ffr/eyetrack key ---"
DB=$(find /data/user_de/0/com.pvr.configuration -name "config.db" 2>/dev/null | head -1)
echo "DB=$DB"
if [ -n "$DB" ]; then
  su -c "sqlite3 $DB 'SELECT * FROM config WHERE key LIKE \"%fovea%\" OR key LIKE \"%ffr%\" OR key LIKE \"%eye%\" OR key LIKE \"%track%\"' 2>&1" | head -20
fi
echo ""
echo "=== PicoEyeTracking app 状态 (被禁用?) ==="
pm list packages -d 2>/dev/null | grep -i eye
echo "--- 包信息 ---"
dumpsys package com.picovr.PicoEyeTracking 2>/dev/null | grep -iE "enabled|User 0|ComponentState|hidden" | head
echo "--- 尝试启用它 ---"
pm enable com.picovr.PicoEyeTracking 2>&1
echo ""
echo "=== setprop enable_ffr 测试(临时) ==="
setprop persist.pvr.config.enable_ffr 1 2>&1
echo "setprop exit=$?"
getprop persist.pvr.config.enable_ffr
echo "done"
