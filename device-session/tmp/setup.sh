#!/system/bin/sh
echo "=== disable magisk builtin zygisk ==="
/data/adb/magisk/magisk --sqlite "INSERT OR REPLACE INTO settings (key,value) VALUES('zygisk','0')"
/data/adb/magisk/magisk --sqlite "SELECT value FROM settings WHERE key='zygisk'"
echo "=== enable zygisk-next (zygisksu) ==="
rm -f /data/adb/modules/zygisksu/disable
echo "zygisksu disable removed"
echo "=== enable vector ==="
rm -f /data/adb/modules/zygisk_vector/disable
echo "vector disable removed"
echo "=== remaining modules ==="
ls /data/adb/modules/
echo "=== done ==="
