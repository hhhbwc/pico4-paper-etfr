#!/system/bin/sh
# Round-trip test getAlgType patch on pxreyetrackingservice(9614), then REAL apply
A=/data/local/tmp/patch_ret1
GAT=0x7480342c8c   # AlgorithmManager::getAlgType, str w8,[x0]
echo "=== safe round-trip ==="
su -c "$A 9614 $GAT 0 str" 2>&1
echo "--- alive? ---"
ps -A 2>/dev/null | grep -c " 9614 "
echo "--- restore ---"
su -c "/data/local/tmp/patch_func restore 9614 0x7480342c8c" 2>&1 || echo "use ret1 restore manually"
# ret1 restore needs its orig file; use a small restore via patch_func won't match format.
echo "done"
