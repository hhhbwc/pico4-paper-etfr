#!/system/bin/sh
echo "=== start a harmless test process ==="
sleep 60 &
TESTPID=$!
echo "test pid=$TESTPID"
echo "=== attach probe (read-only) ==="
/data/local/tmp/ptp $TESTPID
echo "=== probe exit code: $? ==="
kill $TESTPID 2>/dev/null
echo "done"
