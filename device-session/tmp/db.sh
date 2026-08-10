#!/system/bin/sh
DB=/data/user_de/0/com.pvr.configuration/databases/config.db
echo "=== tables ==="
sqlite3 $DB ".tables" 2>&1
echo "=== config entries ==="
sqlite3 $DB "SELECT * FROM config LIMIT 80" 2>&1 | head -80
