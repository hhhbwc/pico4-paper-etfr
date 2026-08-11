#!/system/bin/sh
echo "=== winlimit 模块 ==="
ls -la /data/adb/modules/ 2>/dev/null | grep -iE "win|res|2d|limit"
echo "--- winlimit 内容 ---"
ls -R /data/adb/modules/winlimit 2>/dev/null | head -40
echo ""
echo "=== 2D/平面 应用相关属性 ==="
getprop | grep -iE "2d|2D|flat|window|resize|vr_mode|fov" | head -20
echo ""
echo "=== 当前显示/屏幕配置 ==="
dumpsys display 2>/dev/null | grep -iE "DisplayDeviceInfo|resolution|1600|900|physicalFrame" | head -20
echo ""
echo "=== SurfaceFlinger 图层/配置 ==="
dumpsys SurfaceFlinger 2>/dev/null | grep -iE "1600x900|resolution|Display" | head -15
echo "done"
