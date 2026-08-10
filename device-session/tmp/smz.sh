#!/system/bin/sh
echo "=== enable magisk builtin zygisk ==="
/data/adb/magisk/magisk --sqlite "INSERT OR REPLACE INTO settings (key,value) VALUES('zygisk','1')"
/data/adb/magisk/magisk --sqlite "SELECT value FROM settings WHERE key='zygisk'"
echo "=== remove zygisk-next ==="
rm -rf /data/adb/modules/zygisksu
echo "zygisksu removed"
echo "=== ensure vector enabled ==="
rm -f /data/adb/modules/zygisk_vector/disable
echo "=== remaining modules ==="
ls /data/adb/modules/
echo "=== done ==="
