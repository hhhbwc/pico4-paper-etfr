#!/system/bin/sh
# Try to read raw USB CDC data from the eye tracker (bypassing paper app)
# First check if we can open /dev/bus/usb
DEV=/dev/bus/usb/001/003   # left eye tracker likely
echo "=== device fds open by system ==="
ls -la $DEV 2>&1
echo "=== test open device ==="
# read config descriptor via sysfs won't give data. Try raw:
python3 -c "print('skip')" 2>/dev/null || echo "no python3"
echo "=== is paper holding device? ==="
PID=$(pidof com.bridge.papertracker)
echo "paper pid=$PID"
ls -la /proc/$PID/fd 2>/dev/null | grep "bus/usb"
