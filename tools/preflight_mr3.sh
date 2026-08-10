#!/system/bin/sh
# use full hardcoded hex (no shell arithmetic truncation)
# mrservice(2265) libeyetrackingclient base = 0x7c5c90d000
# GetEyeTrackingData off 0x13584 -> 0x7c5c90d000+0x13584 = 0x7c5c920584
echo "=== read mrservice GetEyeTrackingData (hardcoded full hex) ==="
su -c "/data/local/tmp/ptrace_rw 2265 read 0x7c5c920584" 2>&1
echo ""
echo "=== read openxr_runtime GetEyeTrackingData (base 0x7c6790b000 + 0x13584 = 0x7c6791e584) ==="
su -c "/data/local/tmp/ptrace_rw 2009 read 0x7c6791e584" 2>&1
echo ""
echo "=== try read .rodata (non-exec, more permissive?) mrservice ==="
su -c "/data/local/tmp/ptrace_rw 2265 read 0x7c5c92b000" 2>&1
echo "done"
