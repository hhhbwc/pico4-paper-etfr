#!/system/bin/sh
# Read getAlgType cache to determine active algorithm (Standard=1 / Tobii=2)
# cache at libpxreyetrackingservice.so + 0x61000+0xc = relative to base
# base = 0x7480308000. addr = base + 0x6100c = 0x748036900C? compute on device via python3
echo "=== 找设备 python3 ==="
which python3 2>&1
echo "=== read algType cache byte ==="
su -c "cat /proc/9614/maps | grep libpxreyetrackingservice.so | head -1"
# addr = base + 0x6100c  (the .data.rel.ro or data region holding the cached algType)
echo "=== 尝试读内存 (需 ptrace_rw) ==="
su -c "/data/local/tmp/ptrace_rw 9614 read 0x748036900c" 2>&1
echo "=== trackingmode prop (可能是算法选择依据) ==="
getprop persist.pvr.sdk.trackingmode
getprop persist.pvrservice.trackingmode
getprop persist.pvr.trackingmode 2>/dev/null
echo "done"
