#!/system/bin/sh
echo "=== enable zygisk ==="
/data/adb/magisk/magisk --sqlite "INSERT OR REPLACE INTO settings (key,value) VALUES('zygisk','1')" 2>&1
echo "=== verify ==="
/data/adb/magisk/magisk --sqlite "SELECT * FROM settings" 2>&1
echo "=== done ==="
