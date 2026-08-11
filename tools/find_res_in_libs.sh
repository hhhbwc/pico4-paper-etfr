#!/system/bin/sh
echo "=== systemext 相关 native lib 里含 NS_APP / 分辨率 / window 字符串的 ==="
for so in libpxrguiex libpvrserviceclient libPvr_UnitySDKExt14 libPvr_UESDKExt5 libpxr_xrsdk_native; do
  F=/system/lib64/$so.so
  if [ -f "$F" ]; then
    H=$(su -c "strings $F 2>/dev/null | grep -icE 'NS_APP|NSApp|1602|virtualDisplay|createVirtual|VirtualDisplay|window.*size|2d.*res|flat.*screen'" 2>/dev/null)
    echo "$so : hits=$H"
    [ "$H" -gt 0 ] && su -c "strings $F 2>/dev/null | grep -iE 'NS_APP|NSApp|1602|VirtualDisplay|window.*size|2d.res' | head -8"
  fi
done
echo ""
echo "=== systemext dex 里有没有相关类 (搜索 APK 反编译线索) ==="
APK=/system/priv-app/SystemExt/SystemExt.apk
echo "APK size: $(su -c "ls -la $APK 2>/dev/null | awk '{print \$5}'")"
echo "done"
