# Bully Algorithm Implementation

This is a Python implementation of the Bully Algorithm for leader election in a distributed system.

## Overview

The Bully Algorithm ensures that at any time, there is exactly one process designated as the leader in a network of processes. When a process detects that the leader has failed, it initiates an election to select a new leader.

## Requirements

- Python 3.6 or higher
- No additional packages required (uses only standard library)

## How to Run

### Option 1: Using the Automated Scripts

We provide two scripts to help you run all 5 processes automatically:

#### Multi-Terminal Script (Recommended for testing)

This script will open 5 separate terminal windows, each running a different process:

```bash
./run_bully_algorithm.sh
```

#### Single-Terminal Script

This script will run all 5 processes in the background in a single terminal:

```bash
./run_bully_algorithm_single_terminal.sh
```

The output from each process will be saved to log files (process_1.log, process_2.log, etc.).

### Option 2: Manual Execution

To run the program manually, you need to start multiple instances of the script, each with a different process ID. The process IDs must be between 1 and 5 (as defined by `MAX_PROCESSES`).

#### Step 1: Open multiple terminal windows

You'll need to open 5 terminal windows (one for each process).

#### Step 2: Run the program in each terminal

In each terminal, navigate to the directory containing the script and run:

```bash
python bully_monitor.py <process_id>
```

Where `<process_id>` is a number between 1 and 5.

For example:

Terminal 1:
```bash
python bully_monitor.py 1
```

Terminal 2:
```bash
python bully_monitor.py 2
```

And so on for terminals 3, 4, and 5.

## Testing the Algorithm

### Using the Multi-Terminal Script

1. Run `./run_bully_algorithm.sh`
2. Observe that Process 5 becomes the initial leader
3. Close the terminal running Process 5 to simulate a leader failure
4. Watch as the remaining processes detect the failure and initiate an election
5. Observe that Process 4 becomes the new leader
6. You can continue this process by closing other terminals

### Using the Single-Terminal Script

1. Run `./run_bully_algorithm_single_terminal.sh`
2. Check the log files to observe that Process 5 becomes the initial leader
3. To simulate a leader failure, kill Process 5 using the command shown in the script
4. Watch the log files as the remaining processes detect the failure and initiate an election
5. Observe that Process 4 becomes the new leader
6. You can continue this process by killing other processes

### Manual Testing

- Process 5 will automatically become the initial leader (since it has the highest ID)
- If you close the terminal running process 5, the remaining processes will detect the leader failure and initiate an election
- Process 4 will become the new leader (as it has the highest remaining ID)
- This process continues as processes fail and recover

## How It Works

1. Each process listens on a unique port (BASE_PORT + process_id)
2. Processes communicate via TCP sockets
3. The process with the highest ID that's still alive becomes the leader
4. When a process detects the leader is down, it initiates an election
5. The election process follows the Bully Algorithm rules

## Troubleshooting

- If you get "Address already in use" errors, make sure no other instances are running
- If processes can't communicate, check that your firewall isn't blocking the connections
- Ensure all processes are running on the same machine (localhost) 