#!/system/bin/sh
echo "=== pxreyetrackingservice(9614) 启动/相机配置日志 ==="
su -c "logcat -d 2>/dev/null | grep -iE '9614|pxreyetrack|EyeTrackingService|CameraManager|camera' | grep -viE 'trackingreset|IMU|PCLog' | tail -30"
echo ""
echo "=== 系统所有相机(官方 camera framework) ==="
dumpsys media.camera 2>/dev/null | grep -iE 'Number of cameras|Camera ID|camera.*id' | head -20
echo ""
echo "=== /dev/video* (标准版存在的视频设备) ==="
ls -la /dev/video* 2>/dev/null
echo ""
echo "=== 有没有已加载眼追相机驱动/节点 ==="
find /sys/class/video4linux -iname "*" 2>/dev/null | head -20
echo "done"
