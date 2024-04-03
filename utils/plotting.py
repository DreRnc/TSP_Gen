import matplotlib.pyplot as plt

# Read the text from the file
with open('results/Times.txt', 'r') as file:
    data = file.read()

# Split the text into sections based on the '-----' delimiter
sections = data.strip().split('-----')

# Initialize lists to store number of workers and total times
num_workers = []
total_times = []

# Extract number of workers and total times from each section
for section in sections:
    lines = section.strip().split('\n')
    for line in lines:
        if line.startswith('Number of workers:'):
            num_workers.append(int(line.split()[-1]))
        elif line.startswith('Total time:'):
            total_times.append(int(line.split()[-1]))

# Calculate speedup relative to sequential execution (using 1 worker)
sequential_time = total_times[0]
speedup = [sequential_time / time for time in total_times]

# Plot speedup versus number of workers
plt.plot(num_workers, speedup, marker='o', label='Speedup')
plt.plot(num_workers, num_workers, linestyle='--', color='r', label='Ideal Speedup (x=y)')
plt.xlabel('Number of Workers')
plt.ylabel('Speedup')
plt.title('Speedup vs. Number of Workers')
plt.legend()
plt.grid(True)
plt.show()