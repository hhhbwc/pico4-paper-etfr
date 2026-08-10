#!/system/bin/sh
echo "=== 眼追相关服务/进程 ==="
ps -A 2>/dev/null | grep -iE "eye.*track|pxr.*service" | head
echo "=== 眼追相关prop (完整) ==="
getprop | grep -iE "eye|gaze|track" | grep -vE "af\.|ro\.|sys\.|init\.svc" | head -30
echo "=== 眼追 IPC/socket ==="
ls -la /data/misc/pxr/ 2>/dev/null
find / -iname "*eyetrack*" -o -iname "*gaze*" 2>/dev/null | grep -vE "proc|sys/dev" | head
echo "=== foveated/ETFR 服务接口 ==="
getprop | grep -i foveat
