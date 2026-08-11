#!/system/bin/sh
echo "=== systemext APK 位置 ==="
pm path com.picovr.systemext 2>/dev/null
echo ""
echo "=== systemext 目录 ==="
su -c "ls -la /system/priv-app/com.picovr.systemext/ 2>/dev/null"
echo ""
echo "=== systemext 里哪个文件提到 NS_APP / 分辨率 ==="
APK=$(pm path com.picovr.systemext 2>/dev/null | grep base | sed 's/package://' | head -1)
echo "APK=$APK"
# systemext 可能 native lib 决定分辨率; 找它加载的 so
su -c "cat /proc/5078/maps 2>/dev/null | grep -oE '/[a-zA-Z0-9_./-]+\.so' | sort -u | grep -iE 'pvr|pxr|ns|window|display|surface|ext'" | head -30
echo ""
echo "=== systemext 日志: NS_APP 分辨率怎么定的 ==="
su -c "logcat -d 2>/dev/null | grep -iE 'systemext|NS_APP|NSApp|1602|902|createVirtual|VirtualDisplay' | tail -20"
echo "done"
