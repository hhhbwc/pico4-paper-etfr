#!/system/bin/sh
echo "=== Magisk 模块 ==="
ls /data/adb/modules/ 2>/dev/null
echo ""
echo "=== LSPosed / Xposed 框架 ==="
ls /data/adb/lspd 2>/dev/null && echo "lspd exists"
ls /data/adb/modules/*/ | grep -iE "lsp|xposed" 2>/dev/null
getprop | grep -iE "lsposed|xposed" | head
echo "--- LSPosed manager app? ---"
pm list packages 2>/dev/null | grep -iE "lsposed|xposed|edxposed"
echo ""
echo "=== Zygisk / vector 模块 ==="
ls /data/adb/modules/zygisksu 2>/dev/null && echo "zygisksu exists"
ls /data/adb/modules/zygisk_vector 2>/dev/null && echo "vector exists"
echo ""
echo "=== systemext 是否可写/可替换 (系统 app) ==="
ls -la /system/priv-app/SystemExt/ 2>/dev/null
echo "done"
