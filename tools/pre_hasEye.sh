#!/system/bin/sh
echo "=== pxreyetrackingservice(9614) 当前算法状态 ==="
getprop sys.pxr.trackingservice.algstate
getprop sys.pxr.trackingservice.alg_confidence
echo "=== libpxreyetrackingservice.so base in 9614 ==="
su -c "cat /proc/9614/maps | grep libpxreyetrackingservice.so | head -1 | cut -d'-' -f1"
echo "=== 9614 是否加载 phoenix 算法库 ==="
su -c "cat /proc/9614/maps | grep -iE 'phoenix|eyetrackingservice' | head"
echo "=== 当前 hasEyeCamera 相关日志(有没有被调用过) ==="
su -c "logcat -d 2>/dev/null | grep -iE 'hasEyeCamera|hasEye|eyeCamera|isSupportEye' | tail -10"
echo "done"
