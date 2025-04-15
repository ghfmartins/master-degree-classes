#!/bin/bash

# Script to run all 5 instances of the bully_monitor.py program
# This script will open 5 terminal windows, each running a different process

# Check if bully_monitor.py exists
if [ ! -f "bully_monitor.py" ]; then
    echo "Error: bully_monitor.py not found in the current directory."
    exit 1
fi

# Make sure the script is executable
chmod +x bully_monitor.py

# Function to open a terminal and run a process
run_process() {
    local process_id=$1
    
    # For macOS
    if [[ "$OSTYPE" == "darwin"* ]]; then
        osascript -e "tell app \"Terminal\" to do script \"cd $(pwd) && python3 bully_monitor.py $process_id\""
    # For Linux
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        gnome-terminal -- bash -c "cd $(pwd) && python3 bully_monitor.py $process_id; exec bash"
    # For Windows (if using Git Bash or similar)
    elif [[ "$OSTYPE" == "msys" ]]; then
        start cmd /k "cd $(pwd) && python bully_monitor.py $process_id"
    else
        echo "Unsupported operating system. Please run the processes manually."
        exit 1
    fi
}

echo "Starting Bully Algorithm with 5 processes..."
echo "Process 5 will be the initial leader."

# Run all 5 processes
for i in {1..5}; do
    run_process $i
    # Small delay to prevent all terminals from opening at once
    sleep 1
done

echo "All processes started. You should see 5 terminal windows."
echo "To stop all processes, close each terminal window or press Ctrl+C in each."
echo ""
echo "Testing instructions:"
echo "1. Observe that Process 5 becomes the initial leader"
echo "2. Close the terminal running Process 5 to simulate a leader failure"
echo "3. Watch as the remaining processes detect the failure and initiate an election"
echo "4. Observe that Process 4 becomes the new leader"
echo "5. You can continue this process by closing other terminals" 