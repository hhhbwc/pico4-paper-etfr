#!/system/bin/sh
echo "=== read GetEyeTrackingData entry (should be: sub sp,sp,#0xa0) ==="
su -c "/data/local/tmp/ptrace_rw 2009 read 0x7c6791e584" 2>&1
echo ""
echo "=== also read first 16 bytes of pxr_eyepose struct region? no, read the export stub area ==="
# export stub GetEyeTrackingData_2_0 at 0x16420 -> runtime
su -c "/data/local/tmp/ptrace_rw 2009 read 0x7c67921420" 2>&1
echo "done"
