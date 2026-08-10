#!/system/bin/sh
echo "=== 当前眼动追踪状态 ==="
echo "algstate=$(getprop sys.pxr.trackingservice.algstate)"
echo "alg_confidence=$(getprop sys.pxr.trackingservice.alg_confidence)"
echo "foveation.level=$(getprop persist.pvr.foveation.level)"
echo ""
echo "=== hasEyeCamera hook 状态 (还在吗) ==="
su -c "/data/local/tmp/ptrace_rw 9614 read 0x7480336cdc" 2>&1
echo "=== 眼追服务是否在产数据 (最近日志) ==="
su -c "logcat -d -t 120 2>/dev/null | grep -iE 'EyeTrack|gaze|eyetrack|algstate|confidence|GLASS|wear' | grep -viE 'trackingreset|IMU|PCLog' | tail -15"
echo ""
echo "=== 有没有 Pxr_GetEyeTrackingData 相关查询 (第三方想用眼追) ==="
su -c "logcat -d -t 200 2>/dev/null | grep -iE 'GetEyeTracking|EyeTrackingData|pxr_eyepose|eye_data' | tail -10"
echo "done"
