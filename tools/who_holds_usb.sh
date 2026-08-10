#!/system/bin/sh
echo "=== who holds usb device fds ==="
for pid in /proc/[0-9]*/; do
  p=${pid%/}
  p=${p##*/}
  for fd in $pid/fd/*; do
    t=$(readlink $fd 2>/dev/null)
    case "$t" in
      /dev/bus/usb/*) echo "pid=$p ($(cat /proc/$p/cmdline 2>/dev/null | tr '\0' ' ')) -> $t";;
    esac
  done
done
echo "=== done ==="
