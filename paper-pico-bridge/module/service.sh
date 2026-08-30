#!/system/bin/sh
MODDIR=${0%/*}
RUNDIR=/data/local/tmp/paper-pico-bridge
if [ -f "$MODDIR/disable" ]; then exit 0; fi
mkdir -p "$RUNDIR"
chmod 700 "$RUNDIR"
chmod 755 "$MODDIR/system/bin/paper_bridge_daemon" 2>/dev/null
# No persistent daemon is started until a control-plane/UI exists. Explicit commands
# acquire the daemon lock for bounded USB capture and release it on exit.
exit 0
