#!/system/bin/sh
echo "=== USB buses ==="
ls -la /sys/bus/usb/devices/ 2>/dev/null | head -30
echo "=== all vendor/products ==="
for d in /sys/bus/usb/devices/*/; do
  v=$(cat $d/idVendor 2>/dev/null)
  p=$(cat $d/idProduct 2>/dev/null)
  if [ -n "$v" ]; then
    echo "$d : $v:$p $(cat $d/manufacturer 2>/dev/null) $(cat $d/product 2>/dev/null)"
  fi
done
echo "=== /dev/bus/usb ==="
ls -R /dev/bus/usb/ 2>/dev/null
