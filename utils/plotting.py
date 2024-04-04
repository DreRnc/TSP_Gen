import matplotlib.pyplot as plt
import os

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

def read_imbalance_maxmin(filename):
    """Reads the load times from the given file."""
    with open(filename, 'r') as file:
        data = file.read()

    num_workers = []
    load_imbalances = []

    sections = data.strip().split('-----')
    for section in sections:
        lines = section.strip().split('\n')
        min_load = None
        max_load = None
        for line in lines:
            if line.startswith('Number of workers:'):
                num_workers.append(int(line.split()[-1]))
            elif line.startswith('Min load time among workers'):
                min_load = int(line.split()[-1])
            elif line.startswith('Max load time among workers'):
                max_load = int(line.split()[-1])
        if min_load is not None and max_load is not None:
            imbalance = max_load - min_load
            load_imbalances.append(imbalance)
    return num_workers, load_imbalances

def read_imbalance_std(filename):
    """Reads the load times from the given file
    and calculates the standard deviation of the load times."""
    with open(filename, 'r') as file:
        data = file.read()

    num_workers = []
    load_imbalances = []

    sections = data.strip().split('-----')
    for section in sections:
        lines = section.strip().split('\n')
        for line in lines:
            if line.startswith('Number of workers:'):
                num_workers.append(int(line.split()[-1]))
            elif line.startswith('Std'):
                load_imbalances.append(float(line.split()[-1]))
    return num_workers, load_imbalances

def plot_load_imbalance(country, mode = 'std'):
    # Read load imbalance data for different number of workers
    if mode == 'std':
        read_imbalance = read_imbalance_std
    elif mode == 'maxmin':
        read_imbalance = read_imbalance_maxmin

    num_workers_stat, load_imbalances_stat = read_imbalance(path + f'/results/{country}/time_stat.txt')
    num_workers_dyn, load_imbalances_dyn = read_imbalance(path + f'/results/{country}/time_dyn.txt')

    # Plot load imbalance versus number of workers for static and dynamic parallelism
    plt.plot(num_workers_stat, load_imbalances_stat, marker='o', label='Static Parallelism')
    plt.plot(num_workers_dyn, load_imbalances_dyn, marker='s', label='Dynamic Parallelism')
    plt.xlabel('Number of Workers')
    plt.ylabel('Load Imbalance')
    plt.title(f'Load Imbalance vs. Number of Workers - {country.capitalize()}')
    plt.legend()

    # Customize ticks on x-axis
    plt.xticks(num_workers_stat)

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/load_imbalance_plot_{country}.png')

    # Show the plot
    plt.show()


def plot_speedup(country):
    # Read sequential total time from the sequential time file
    seq_num_workers, seq_total_times = read_times(path + f'/results/{country}/time_seq.txt')
    sequential_time = seq_total_times[0]  # Sequential time is the first entry

    # Read total times for static and dynamic parallelism
    stat_num_workers, stat_total_times = read_times(path + f'/results/{country}/time_stat.txt')
    dyn_num_workers, dyn_total_times = read_times(path + f'/results/{country}/time_dyn.txt')

    # Calculate speedup for static and dynamic parallelism
    stat_speedup = [sequential_time / time for time in stat_total_times]
    dyn_speedup = [sequential_time / time for time in dyn_total_times]

    # Plot speedup versus number of workers for static and dynamic parallelism
    plt.plot(stat_num_workers, stat_speedup, marker='o', label='Static Parallelism')
    plt.plot(dyn_num_workers, dyn_speedup, marker='s', label='Dynamic Parallelism')
    
    # Plot the diagonal
    max_workers = max(max(stat_num_workers), max(dyn_num_workers))
    plt.plot(range(max_workers + 1), range(max_workers + 1), linestyle='--', color='r', label='Ideal Speedup (x=y)')
    
    plt.xlabel('Number of Workers')
    plt.ylabel('Speedup')
    plt.title(f'Speedup vs. Number of Workers - {country.capitalize()}')
    plt.legend()
    
    # Customize ticks on x-axis
    plt.xticks(stat_num_workers + [max_workers], labels=[str(worker) if worker != max_workers else f'{worker} (Hyperthreading)' for worker in stat_num_workers + [max_workers]])
    
    # Remove the grid
    plt.grid(False)

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/speedup_plot_{country}.png')

    # Show the plot
    plt.show()


path = os.getcwd()
countries = ['italy', 'luxembourg', 'canada']
for country in countries:
    plot_speedup(country)
    plot_load_imbalance(country)
