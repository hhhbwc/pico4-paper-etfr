#!/system/bin/sh
echo "=== CPU core freq ==="
for c in cpu0 cpu4 cpu6; do
  d=/sys/devices/system/cpu/$c/cpufreq
  [ -f $d/scaling_max_freq ] && echo "$c max=$(cat $d/scaling_max_freq) min=$(cat $d/scaling_min_freq) cur=$(cat $d/scaling_cur_freq) gov=$(cat $d/scaling_governor)"
done
echo "=== GPU freq ==="
find /sys/class/kgsl /sys/kernel/gpu -name "*freq*" -o -name "gpuclk" 2>/dev/null | head -20
echo "=== GPU current ==="
for f in /sys/class/kgsl/kgsl-3d0/gpuclk /sys/class/kgsl/kgsl-3d0/max_gpuclk /sys/class/kgsl/kgsl-3d0/devfreq/cur_freq; do
  [ -f $f ] && echo "$f = $(cat $f 2>/dev/null)"
done
