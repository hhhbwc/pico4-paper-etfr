#!/system/bin/sh
echo "=== systemext 日志: AppRecord size (非系统 app 实际分辨率) ==="
su -c "logcat -d 2>/dev/null | grep -iE 'SYSTEM_EXT' | grep -iE 'AppRecord|size|orientation|windowLayout' | tail -25"
echo ""
echo "=== 当前设置 virtual_density ==="
settings get global systemext.settings.virtual_density 2>/dev/null
echo "=== 当前有 2D app 跑着吗 (NS_APP 显示) ==="
dumpsys display 2>/dev/null | grep -iE "NS_APP" | head
echo "done"
