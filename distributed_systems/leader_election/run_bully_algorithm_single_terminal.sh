#!/bin/bash

# Script to run all 5 instances of the bully_monitor.py program in a single terminal
# This script will run all processes in the background

# Check if bully_monitor.py exists
if [ ! -f "bully_monitor.py" ]; then
    echo "Error: bully_monitor.py not found in the current directory."
    exit 1
fi

# Make sure the script is executable
chmod +x bully_monitor.py

echo "Starting Bully Algorithm with 5 processes..."
echo "Process 5 will be the initial leader."
echo "All processes will run in the background."
echo "Press Ctrl+C to stop all processes."

# Array to store process IDs
declare -a pids

# Run all 5 processes in the background
for i in {1..5}; do
    python3 bully_monitor.py $i > process_${i}.log 2>&1 &
    pids+=($!)
    echo "Started Process $i with PID: ${pids[-1]}"
    # Small delay to prevent all processes from starting at once
    sleep 1
done

echo ""
echo "All processes started. Check the log files for output:"
echo "process_1.log, process_2.log, process_3.log, process_4.log, process_5.log"
echo ""
echo "Testing instructions:"
echo "1. Observe that Process 5 becomes the initial leader"
echo "2. To simulate a leader failure, kill Process 5: kill ${pids[4]}"
echo "3. Watch as the remaining processes detect the failure and initiate an election"
echo "4. Observe that Process 4 becomes the new leader"
echo "5. You can continue this process by killing other processes"
echo ""
echo "To stop all processes, press Ctrl+C"

# Function to clean up processes on exit
cleanup() {
    echo "Stopping all processes..."
    for pid in "${pids[@]}"; do
        kill $pid 2>/dev/null
    done
    exit 0
}

# Set up trap to catch Ctrl+C
trap cleanup SIGINT

# Keep the script running
while true; do
    sleep 1
done 