#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd
import sys

# Check if a filename is provided as a command-line argument
if len(sys.argv) < 2:
    print("Usage: python plot.py <filename.csv>")
    sys.exit(1)

filename = sys.argv[1]

# Read the CSV file
df = pd.read_csv(filename)

# Create a figure with subplots
fig, axs = plt.subplots(3, 1, figsize=(10, 12), sharex=True)

# Plot the state x
axs[0].plot(df['t'], df['x'], 'b-', label='State (x)')
axs[0].set_ylabel('State x')
axs[0].grid(True)
axs[0].legend()

# Plot the control u
axs[1].plot(df['t'], df['u'], 'g-', label='Control (u)')
axs[1].set_ylabel('Control u')
axs[1].grid(True)
axs[1].legend()

# Plot the costate lambda
axs[2].plot(df['t'], df['lambda'], 'r-', label='Costate (λ)')
axs[2].set_ylabel('Costate λ')
axs[2].set_xlabel('Time t')
axs[2].grid(True)
axs[2].legend()

# Add a global title
plt.suptitle('Optimal Control Problem Solution')

# Adjust the layout
plt.tight_layout()

# Display the graph
plt.show() 