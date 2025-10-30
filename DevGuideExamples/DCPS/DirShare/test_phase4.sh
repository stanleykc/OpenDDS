#!/bin/bash

# Phase 4 manual test script
# Tests real-time file creation propagation

set -e

# Setup test directories
mkdir -p /tmp/dirshare_test1 /tmp/dirshare_test2
rm -f /tmp/dirshare_test1/* /tmp/dirshare_test2/*

echo "Starting DirShare instance 1 in /tmp/dirshare_test1..."
source /Users/stanleyk/dev/OpenDDS/setenv.sh
./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_test1 > /tmp/dirshare1.log 2>&1 &
PID1=$!

echo "Waiting for instance 1 to initialize..."
sleep 3

echo "Starting DirShare instance 2 in /tmp/dirshare_test2..."
./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_test2 > /tmp/dirshare2.log 2>&1 &
PID2=$!

echo "Waiting for both instances to discover each other..."
sleep 5

echo ""
echo "=== Creating test file in instance 1 ==="
echo "This is a Phase 4 test file" > /tmp/dirshare_test1/phase4test.txt
echo "File created at $(date)"

echo "Waiting for file to propagate (5 seconds)..."
sleep 5

echo ""
echo "=== Verifying file propagation ==="
echo "Instance 1 directory:"
ls -la /tmp/dirshare_test1/

echo ""
echo "Instance 2 directory:"
ls -la /tmp/dirshare_test2/

echo ""
if [ -f /tmp/dirshare_test2/phase4test.txt ]; then
    echo "SUCCESS! File propagated to instance 2"
    echo "Content:"
    cat /tmp/dirshare_test2/phase4test.txt
else
    echo "FAIL! File did not propagate to instance 2"
fi

echo ""
echo "=== Checking logs ==="
echo "Instance 1 log (last 20 lines):"
tail -20 /tmp/dirshare1.log | grep -E "(CREATE|phase4test)" || echo "No CREATE events found"

echo ""
echo "Instance 2 log (last 20 lines):"
tail -20 /tmp/dirshare2.log | grep -E "(CREATE|phase4test)" || echo "No CREATE events received"

echo ""
echo "Cleaning up..."
kill $PID1 $PID2 2>/dev/null || true
wait 2>/dev/null || true

echo "Test complete!"
