#!/system/bin/sh
echo "=== /dev 下所有 tty 和 serial ==="
ls /dev/ | grep -iE "tty|serial|usb"
echo "=== FTDI 内核模块 ==="
lsmod 2>/dev/null | grep -i ftdi
cat /proc/modules 2>/dev/null | grep -i ftdi
echo "=== 找 vendor 0425 (FTDI) 设备 ==="
find /sys/bus/usb -name "idVendor" -exec sh -c 'echo "$1: $(cat $1)"' _ {} \; 2>/dev/null | grep "0425"
echo "=== paper app 打开的文件(看它读啥) ==="
PID=$(pidof com.bridge.papertracker)
echo "PID=$PID"
ls -la /proc/$PID/fd 2>/dev/null | grep -iE "tty|usb|serial" 
cat /proc/$PID/maps 2>/dev/null | grep -iE "serial|usb|ftdi" | head
