#!/system/bin/sh
echo "=== Magisk 模块 (su) ==="
su -c "ls /data/adb/modules/ 2>/dev/null"
echo ""
echo "=== LSPosed / Xposed 痕迹 ==="
su -c "ls /data/adb/lspd 2>/dev/null && echo LSPD_DIR"
su -c "ls /data/adb/modules/*/ | grep -iE 'lsp|xposed|rine|zygisk' 2>/dev/null"
getprop | grep -iE "lsposed|xposed|zygisk" | head
echo "--- LSPosed app? ---"
pm list packages 2>/dev/null | grep -iE "lsposed|org.lsposed|xposed"
echo ""
echo "=== zygisk 状态 ==="
su -c "/data/adb/magisk/magisk --sqlite 'SELECT * FROM settings' 2>/dev/null"
echo "done"
