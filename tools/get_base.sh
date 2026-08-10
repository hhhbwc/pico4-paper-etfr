#!/system/bin/sh
echo "=== openxr_runtime(2009) libopenxr_api.so base ==="
su -c "cat /proc/2009/maps | grep -E 'libopenxr_api.so'" 2>&1
echo ""
echo "=== GOT region around 0x62000 (data segment) ==="
su -c "cat /proc/2009/maps | grep -E 'r--p|rw-p' | grep -iE 'openxr_api'" 2>&1
echo ""
echo "=== vrshell(2456) libopenxr_api.so base ==="
su -c "cat /proc/2456/maps | grep -E 'libopenxr_api.so'" 2>&1
echo "done"
