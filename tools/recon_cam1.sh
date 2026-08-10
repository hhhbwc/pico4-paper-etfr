#!/system/bin/sh
echo "=== 眼追相机相关所有符号 ==="
strings /system/lib64/libpxreyetrackingservice.so 2>/dev/null | grep -iE "camera|stream|frame|open|close|start|stop|dev/|/dev|mmap|buffer|image|inject" | head -40
echo ""
echo "=== 眼追相关 dev 节点 ==="
ls -la /dev/ 2>/dev/null | grep -iE "camera|isp|csi|eye|gaze|pvr|pxr"
echo ""
echo "=== CameraManager / camera hal ==="
find /vendor /system -iname "*camera*cam*" -o -iname "*camx*" 2>/dev/null | head
