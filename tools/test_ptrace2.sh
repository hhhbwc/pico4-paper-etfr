#!/system/bin/sh
chmod 755 /data/local/tmp/ptp2
sleep 30 &
TESTPID=$!
echo "test pid=$TESTPID"
/data/local/tmp/ptp2 $TESTPID
echo "exit=$?"
kill $TESTPID 2>/dev/null
