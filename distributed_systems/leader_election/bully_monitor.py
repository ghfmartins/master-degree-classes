#!/usr/bin/env python3
import socket
import threading
import time
import sys
import random

# Constants
MAX_PROCESSES = 5
BASE_PORT = 5000
BUFFER_SIZE = 1024

class BullyMonitor:
    def __init__(self, process_id):
        self.process_id = process_id
        self.leader_id = -1
        self.election_in_progress = False
        self.waiting_for_ok = False
        self.received_ok = False
        self.running = True
        
        # Create and start listener thread
        self.listener_thread = threading.Thread(target=self.listener)
        self.listener_thread.daemon = True
        self.listener_thread.start()
        
        # Create and start monitor thread
        self.monitor_thread = threading.Thread(target=self.monitor_leader)
        self.monitor_thread.daemon = True
        self.monitor_thread.start()
    
    def send_message(self, target_id, message):
        """Send a message to a specific process"""
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(1)  # 1 second timeout
                sock.connect(('127.0.0.1', BASE_PORT + target_id))
                data = f"{self.process_id}:{message}"
                sock.sendall(data.encode())
                return True
        except (socket.timeout, ConnectionRefusedError, OSError):
            return False
    
    def broadcast(self, message):
        """Send a message to all other processes"""
        for i in range(1, MAX_PROCESSES + 1):
            if i != self.process_id:
                self.send_message(i, message)
    
    def listener(self):
        """Thread that listens for incoming messages"""
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('127.0.0.1', BASE_PORT + self.process_id))
        server_socket.listen(5)
        
        while self.running:
            try:
                server_socket.settimeout(1)  # 1 second timeout
                client_socket, addr = server_socket.accept()
                
                with client_socket:
                    data = client_socket.recv(BUFFER_SIZE).decode()
                    if not data:
                        continue
                    
                    sender_id, message = data.split(':', 1)
                    sender_id = int(sender_id)
                    
                    if message == "ELECTION":
                        print(f"📨 Process {self.process_id} received ELECTION from {sender_id}")
                        
                        # If I'm the leader, ignore election
                        if self.leader_id == self.process_id:
                            print(f"👑 I am the current leader. Ignoring ELECTION from {sender_id}")
                        else:
                            self.send_message(sender_id, "OK")
                            
                            if not self.election_in_progress:
                                time.sleep(1)
                                self.initiate_election()
                    
                    elif message == "OK":
                        print(f"✅ Process {self.process_id} received OK from {sender_id}")
                        self.received_ok = True
                    
                    elif message == "LEADER":
                        print(f"👑 New leader announced: {sender_id}")
                        self.leader_id = sender_id
                        self.election_in_progress = False
                        self.waiting_for_ok = False
                        self.received_ok = False
            
            except socket.timeout:
                continue
            except Exception as e:
                print(f"Error in listener: {e}")
                time.sleep(1)
    
    def initiate_election(self):
        """Start the election process"""
        if self.election_in_progress:
            return
        
        self.election_in_progress = True
        self.waiting_for_ok = True
        self.received_ok = False
        
        print(f"🔔 Process {self.process_id} started election")
        
        # Send ELECTION to all processes with higher IDs
        for i in range(self.process_id + 1, MAX_PROCESSES + 1):
            self.send_message(i, "ELECTION")
        
        # Wait for OK responses for 3 seconds
        start_time = time.time()
        while time.time() - start_time < 3:
            if self.received_ok:
                break
            time.sleep(0.1)
        
        self.waiting_for_ok = False
        
        if not self.received_ok:
            # If no one responded, declare myself as leader
            print(f"👑 Process {self.process_id} declared itself as LEADER")
            self.leader_id = self.process_id
            self.broadcast("LEADER")
            self.election_in_progress = False
        else:
            # Wait for the new leader announcement
            print(f"⌛ Process {self.process_id} waiting for new leader...")
            wait_start = time.time()
            while time.time() - wait_start < 5:
                if not self.election_in_progress:
                    return  # Leader was announced
                time.sleep(0.1)
            
            # If no one announced, try again
            print(f"⏱️ No one announced leader. Process {self.process_id} restarting election.")
            self.election_in_progress = False
            self.initiate_election()
    
    def monitor_leader(self):
        """Monitor the leader's health"""
        while self.running:
            time.sleep(5)
            
            if self.leader_id == self.process_id:
                continue
            
            # Try to connect to the leader
            if not self.send_message(self.leader_id, "PING"):
                print(f"⚠️ Failed to contact leader {self.leader_id}! Starting new election...")
                self.initiate_election()
    
    def stop(self):
        """Stop the monitor"""
        self.running = False

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <process_id>")
        sys.exit(1)
    
    process_id = int(sys.argv[1])
    if process_id < 1 or process_id > MAX_PROCESSES:
        print(f"Process ID must be between 1 and {MAX_PROCESSES}")
        sys.exit(1)
    
    monitor = BullyMonitor(process_id)
    
    # If this is the highest ID process, it becomes the initial leader
    if process_id == MAX_PROCESSES:
        monitor.leader_id = process_id
        print(f"👑 Process {process_id} started as LEADER")
        monitor.broadcast("LEADER")
    
    try:
        # Keep the main thread alive
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down...")
        monitor.stop()
        sys.exit(0)

if __name__ == "__main__":
    main() 