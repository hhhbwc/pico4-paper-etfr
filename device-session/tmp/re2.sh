#!/system/bin/sh
echo "=== pxreyetrackingservice rc (服务配置) ==="
cat /system/etc/init/pxreyetrackingservice.rc 2>/dev/null
echo "=== eyecalib 数据 ==="
ls -la /data/misc/pxr/eyecalib/ 2>/dev/null
echo "=== 眼追服务相关 prop 完整列表 ==="
getprop | grep -iE "pxreyetrack|eyetrack|gaze" 
echo "=== 是否有眼追数据 socket/dev ==="
find /dev -iname "*eye*" -o -iname "*gaze*" 2>/dev/null
ls /dev/ | grep -iE "pvr|pxr|eye"
