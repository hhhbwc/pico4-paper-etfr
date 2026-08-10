#!/system/bin/sh
echo "=== uid ==="
id
echo "=== maps readable? ==="
ls -la /proc/9614/maps 2>&1
echo "=== head of maps ==="
head -5 /proc/9614/maps 2>&1
echo "=== try su ==="
su -c "cat /proc/9614/maps | grep -oE '[a-zA-Z0-9_./+-]+\.so' | sort -u | head -50" 2>&1
echo "done"
