#!/system/bin/sh
MODDIR=${0%/*}
if [ -f "$MODDIR/disable" ]; then exit 0; fi
mkdir -p /data/local/tmp/paper-pico-bridge
chmod 755 "$MODDIR/system/bin/paper_bridge_daemon" 2>/dev/null
exec "$MODDIR/system/bin/paper_bridge_daemon" >>/data/local/tmp/paper-pico-bridge/daemon.log 2>&1
