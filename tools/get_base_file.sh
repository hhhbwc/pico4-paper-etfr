#!/system/bin/sh
su -c "cat /proc/2009/maps | grep libeyetrackingclient.pxr.so | head -1 | cut -d'-' -f1" > /data/local/tmp/openxr_base.txt 2>&1
cat /data/local/tmp/openxr_base.txt
