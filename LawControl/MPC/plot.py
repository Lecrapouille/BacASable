#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd
import sys

# Check if a filename is provided as a command-line argument
if len(sys.argv) < 2:
    print("Usage: python plot.py <filename.csv>")
    sys.exit(1)

filename = sys.argv[1]

# Read the CSV file with semicolon separator
df = pd.read_csv(filename, sep=';', skiprows=1)

# Create a figure with subplots
fig, axs = plt.subplots(4, 1, figsize=(10, 12), sharex=True)

# Plot the position
axs[0].plot(df['Time(s)'], df['RefPos(m)'], 'b--', label='Target (RefPos)')
axs[0].plot(df['Time(s)'], df['Pos(m)'], 'b-', label='Observed (Pos)')
axs[0].set_ylabel('Position (m)')
axs[0].grid(True)
axs[0].legend()

# Plot the speed
axs[1].plot(df['Time(s)'], df['RefVel(m/s)'], 'g--', label='Target (RefVel)')
axs[1].plot(df['Time(s)'], df['Speed(m/s)'], 'g-', label='Observed (Speed)')
axs[1].set_ylabel('Speed (m/s)')
axs[1].grid(True)
axs[1].legend()

# Plot the acceleration
axs[2].plot(df['Time(s)'], df['RefAcc(m/s²)'], 'r--', label='Target (RefAcc)')
axs[2].plot(df['Time(s)'], df['Acc(m/s²)'], 'r-', label='Observed (Acc)')
axs[2].set_ylabel('Acceleration (m/s²)')
axs[2].grid(True)
axs[2].legend()

# Plot the command
axs[3].plot(df['Time(s)'], df['Command(N)'], 'k-')
axs[3].set_ylabel('Command (N)')
axs[3].set_xlabel('Time (s)')
axs[3].grid(True)

# Add a global title
plt.suptitle('Simulation MPC for a vehicle')

# Adjust the layout
plt.tight_layout()

# Display the graph
plt.show() 