#!/system/bin/sh
# stop paper completely + prevent autostart, then snapshot usb
echo "=== force stop paper ==="
am force-stop com.bridge.papertracker
sleep 2
# try to prevent resurrection: disable via magisk module toggle
echo "=== check paper again ==="
pidof com.bridge.papertracker || echo "paper stopped"
echo "=== temporarily disable paper_autostart module ==="
if [ -d /data/adb/modules/paper_autostart ]; then
  touch /data/adb/modules/paper_autostart/disable
  echo "paper_autostart disabled"
fi
echo "=== ensure boot receiver won't restart (can't easy) - just proceed ==="
