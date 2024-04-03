import matplotlib.pyplot as plt
import os

path = os.getcwd()

def read_times(filename):
    """Reads the total times from the given file."""
    with open(filename, 'r') as file:
        data = file.read()

    num_workers = []
    total_times = []

    sections = data.strip().split('-----')
    for section in sections:
        lines = section.strip().split('\n')
        for line in lines:
            if line.startswith('Number of workers:'):
                num_workers.append(int(line.split()[-1]))
            elif line.startswith('Total time:'):
                total_times.append(int(line.split()[-1]))

    return num_workers, total_times

# Read sequential total time from the sequential time file
seq_num_workers, seq_total_times = read_times(path + '/results/time_seq.txt')
sequential_time = seq_total_times[0]  # Sequential time is the first entry

# Read total times for static and dynamic parallelism
stat_num_workers, stat_total_times = read_times(path + '/results/time_stat.txt')
dyn_num_workers, dyn_total_times = read_times(path + '/results/time_dyn.txt')

# Calculate speedup for static and dynamic parallelism
stat_speedup = [sequential_time / time for time in stat_total_times]
dyn_speedup = [sequential_time / time for time in dyn_total_times]

# Plot speedup versus number of workers for static and dynamic parallelism
plt.plot(stat_num_workers, stat_speedup, marker='o', label='Static Parallelism')
plt.plot(dyn_num_workers, dyn_speedup, marker='s', label='Dynamic Parallelism')
plt.plot(seq_num_workers, seq_num_workers, linestyle='--', color='r', label='Ideal Speedup (x=y)')
plt.xlabel('Number of Workers')
plt.ylabel('Speedup')
plt.title('Speedup vs. Number of Workers')
plt.legend()
plt.grid(True)

# Create 'images' folder if it doesn't exist
if not os.path.exists(path + '/images'):
    os.makedirs(path + '/images')

# Save the plot as an image
plt.savefig(path + '/images/speedup_plot.png')

# Show the plot
plt.show()
