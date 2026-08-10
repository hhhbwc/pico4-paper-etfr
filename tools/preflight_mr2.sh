#!/system/bin/sh
# Preflight v2: correct hex address + read mrservice GetEyeTrackingData code
BASE=$(su -c "cat /proc/2265/maps | grep libeyetrackingclient.pxr.so | head -1 | cut -d'-' -f1")
echo "base = 0x$BASE"
# GetEyeTrackingData offset 0x13584
GED=$(printf '0x%x' $((0x$BASE + 0x13584)))
echo "GetEyeTrackingData runtime = $GED"
echo "--- read first 3 words ---"
su -c "/data/local/tmp/ptrace_rw 2265 read $GED" 2>&1
su -c "/data/local/tmp/ptrace_rw 2265 read 0x$(printf '%x' $((0x$BASE + 0x13584 + 4)))" 2>&1
su -c "/data/local/tmp/ptrace_rw 2265 read 0x$(printf '%x' $((0x$BASE + 0x13584 + 8)))" 2>&1
echo "--- compare: openxr_runtime(2009) same lib read ---"
BASE2=$(su -c "cat /proc/2009/maps | grep libeyetrackingclient.pxr.so | head -1 | cut -d'-' -f1")
echo "openxr base=0x$BASE2"
su -c "/data/local/tmp/ptrace_rw 2009 read 0x$(printf '%x' $((0x$BASE2 + 0x13584)))" 2>&1
echo "done"
