#!/system/bin/sh
echo "=== 验证 hasEyeCamera binder 调用现在返回什么 ==="
# hasEyeCamera takes a target (bool out). Try transact codes.
for code in 1 2 3 4 5 6 7 8 9 10; do
  echo "--- code=$code ---"
  service call pxreyetrackingservice $code 2>&1 | head -3
done
echo ""
echo "=== 眼追服务当前算法/相机状态 ==="
getprop sys.pxr.trackingservice.algstate
getprop sys.pxr.trackingservice.alg_confidence
echo "=== 有没有因为 hasEyeCamera=true 触发的日志 ==="
su -c "logcat -d -t 100 2>/dev/null | grep -iE '9633| 9614 |EyeTracking|hasEye|OpenCamera|StartAlg' | tail -20"
echo "done"
