#!/system/bin/sh
# find which processes load eyetrack-related libs (consumers of eye data)
echo "=== processes loading libeyetrackingclient / libopenxr_api / libpxreyetrackingservice ==="
for pid in $(ps -A | awk '{print $2}'); do
  if grep -qa "libeyetrackingclient" /proc/$pid/maps 2>/dev/null; then
    echo "pid=$pid cmd=[$(cat /proc/$pid/cmdline 2>/dev/null | tr '\0' ' ')] ::: libeyetrackingclient"
  fi
done
echo "--- libopenxr_api ---"
for pid in $(ps -A | awk '{print $2}'); do
  if grep -qa "libopenxr_api" /proc/$pid/maps 2>/dev/null; then
    echo "pid=$pid cmd=[$(cat /proc/$pid/cmdline 2>/dev/null | tr '\0' ' ')] ::: libopenxr_api"
  fi
done
echo "--- libpxreyetrackingservice loaded in ---"
for pid in $(ps -A | awk '{print $2}'); do
  if grep -qa "libpxreyetrackingservice" /proc/$pid/maps 2>/dev/null; then
    echo "pid=$pid cmd=[$(cat /proc/$pid/cmdline 2>/dev/null | tr '\0' ' ')] ::: libpxreyetrackingservice"
  fi
done
echo done
