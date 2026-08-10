#!/system/bin/sh
# read GOT entry of Pxr_GetEyeTrackingData in openxr_runtime via ptp2
# libopenxr_api.so base 0x7cd26da000, GOT slot 0x62ef0 => 0x7cd273def0
echo "=== attach openxr_runtime(2009) read GOT slot 0x7cd273def0 ==="
su -c "/data/local/tmp/ptp2 2009 0x7cd273def0" 2>&1
echo "exit=$?"
echo ""
echo "=== also peek the stub code at 0x7cd26fb988 ==="
su -c "/data/local/tmp/ptp2 2009 0x7cd26fb988" 2>&1
echo "exit=$?"
echo "done"
