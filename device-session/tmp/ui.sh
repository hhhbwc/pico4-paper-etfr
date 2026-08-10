#!/system/bin/sh
echo "=== USB 设备详细 (0425 / esp32) ==="
ls /dev/bus/usb/001/ 2>/dev/null
echo "=== 设备描述符 ==="
for dev in /sys/bus/usb/devices/*/; do
  v=$(cat $dev/idVendor 2>/dev/null)
  if [ "$v" = "0425" ] || [ "$v" = "303a" ]; then
    echo "--- $dev ---"
    echo "  vendor=$v product=$(cat $dev/idProduct 2>/dev/null)"
    echo "  manufacturer=$(cat $dev/manufacturer 2>/dev/null)"
    echo "  product=$(cat $dev/product 2>/dev/null)"
    echo "  interfaces:"
    for intf in $dev*/; do
      [ -d "$intf" ] && [ -f "$intf/bInterfaceClass" ] && echo "    $(basename $intf) class=$(cat $intf/bInterfaceClass 2>/dev/null)/$(cat $intf/bInterfaceSubClass 2>/dev/null)"
    done
  fi
done
