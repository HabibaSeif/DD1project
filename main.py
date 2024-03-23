import matplotlib.pyplot as plt
import numpy as np

# Define the file name
file_name = "simulation.sim"

# Load data from the .sim file
data = np.loadtxt(file_name, dtype={'names': ('timestamps', 'signals', 'values'),
                                     'formats': ('f8', 'S2', 'f8')})

timestamps = data['timestamps']
signals = data['signals'].astype(str)
values = data['values']

# Plot the waveform
plt.figure(figsize=(10, 6))
for signal in set(signals):
    signal_indices = np.where(signals == signal)[0]
    signal_timestamps = timestamps[signal_indices]
    signal_values = values[signal_indices]
    plt.step(signal_timestamps, signal_values, label=signal.decode())

plt.xlabel('Time')
plt.ylabel('Signal Value')
plt.title('Waveform Visualization')
plt.legend()
plt.grid(True)
plt.show()
