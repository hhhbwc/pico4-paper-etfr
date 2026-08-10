#!/system/bin/sh
# query magisk zygisk setting
MAGISK_DB=/data/adb/magisk.db
echo "=== settings table ==="
/data/adb/magisk/magisk --sqlite "SELECT * FROM settings" 2>&1
echo "=== done ==="
