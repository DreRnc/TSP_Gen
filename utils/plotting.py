import matplotlib.pyplot as plt
import os
from matplotlib.patches import Rectangle

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

def read_imbalance(filename, mode = 'perc'):
    """Reads the load times from the given file and calculates the max - mean load imbalance."""
    with open(filename, 'r') as file:
        data = file.read()

    num_workers = []
    load_imbalances = []

    sections = data.strip().split('-----')
    for section in sections:
        lines = section.strip().split('\n')
        max_load = None
        mean_load = None
        for line in lines:
            if line.startswith('Number of workers:'):
                num_workers.append(int(line.split()[-1]))
            elif line.startswith('Max load (averaged over generations):'):
                # int conversion doesn't work: ValueError: invalid literal for int() with base 10: '8.57768e+06'
                max_load = float(line.split()[-3])
            elif line.startswith('Mean load (averaged over generations):'):
                mean_load = float(line.split()[-3])
        if max_load is not None and mean_load is not None:
            if mode == 'perc':
                imbalance = (max_load - mean_load) / mean_load * 100
            elif mode == 'abs_val':
                imbalance = max_load - mean_load
            load_imbalances.append(imbalance)
    return num_workers, load_imbalances

def read_serial_fraction(filename):
    with open(filename, 'r') as file:
        data = file.read()

    num_workers = []
    serial_fraction = []

    sections = data.strip().split('-----')
    for section in sections:
        lines = section.strip().split('\n')
        serial_time = None
        parallel_time = None
        for line in lines:
            if line.startswith('Number of workers:'):
                num_workers.append(int(line.split()[-1]))
            elif line.startswith('Total serial time (including initialization):'):
                serial_time = int(line.split()[-1])
            elif line.startswith('Total non-serial time:'):
                parallel_time = int(line.split()[-1])
        if serial_time is not None and parallel_time is not None:
            serial_fraction.append(serial_time / (serial_time + parallel_time) * 100)
    return num_workers, serial_fraction

def read_overhead(filename, num_worker, n_gen):
    # Return one tuple for the worker = num_worker
    # Extract average max load, total imbalance, total serial time, total non serial time, total time
    with open(filename, 'r') as file:
        data = file.read()

    workers = None

    sections = data.strip().split('-----')
    for section in sections:
        lines = section.strip().split('\n')
        max_load = None
        mean_load = None
        serial_time = None
        total_time = None
        for line in lines:
            if line.startswith('Number of workers:'):
                workers = int(line.split()[-1])
            elif line.startswith('Max load (averaged over generations):'):
                max_load = float(line.split()[-3])
            elif line.startswith('Total load imbalance'):
                total_imbalance = float(line.split()[-1])
            elif line.startswith('Total serial time (including initialization):'):
                serial_time = int(line.split()[-1])
            elif line.startswith('Total non-serial time:'):
                non_serial_time = int(line.split()[-1])
            elif line.startswith('Total time:'):
                total_time = int(line.split()[-1])
        if workers == num_worker and max_load is not None and total_imbalance is not None and serial_time is not None and total_time is not None:
            overhead = non_serial_time - n_gen * max_load
            return [total_time, overhead, total_imbalance, serial_time]

def plot_load_imbalance(country, size, mode = 'perc', max_y = None):
    # Plot the load imbalance for static and dynamic parallelism
    # What I want is a bar plot with the number of workers on the x-axis and the load imbalance on the y-axis
    # There are two bars for each number of workers: one for static and one for dynamic parallelism (not fastflow)
    # Just plot the bar for workers 2 and max_workers

    # Read load imbalance for static and dynamic parallelism
    stat_num_workers, stat_load_imbalances = read_imbalance(path + f'/results/{country}/time_stat_{size}.txt', mode)
    dyn_num_workers, dyn_load_imbalances = read_imbalance(path + f'/results/{country}/time_dyn_{size}.txt', mode)

    mode_name = 'Percentage' if mode == 'perc' else 'Absolute Value'

    stat_selected_load_imbalances = [stat_load_imbalances[stat_num_workers.index(2)], stat_load_imbalances[stat_num_workers.index(max(stat_num_workers))]]
    dyn_selected_load_imbalances = [dyn_load_imbalances[dyn_num_workers.index(2)], dyn_load_imbalances[dyn_num_workers.index(max(dyn_num_workers))]]

    # Plot load imbalance versus number of workers for static and dynamic parallelism
    plt.bar( [0, 1], stat_selected_load_imbalances, width=0.4, align='center', label='Static Parallelism')
    plt.bar( [0.4, 1.4], dyn_selected_load_imbalances, width=0.4, align='center', label='Dynamic Parallelism')

    plt.xlabel('Number of Workers')
    plt.ylabel(f'Load Imbalance ({mode_name})')
    plt.title(f'Load Imbalance vs. Number of Workers - {country.capitalize()} ({size})')
    plt.legend()

    # Customize ticks on x-axis
    plt.xticks([0.2, 1.2], labels=['2', f"{max(stat_num_workers)}"])
    if max_y is not None:
        plt.ylim(0, max_y)
    plt.tight_layout()

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/load_imbalance_plot_{country}_{size}.png')

    # Show the plot
    plt.show()

def plot_speedup(country, size):
    f = 0.00057
    # Read sequential total time from the sequential time file
    seq_num_workers, seq_total_times = read_times(path + f'/results/{country}/time_seq_{size}.txt')
    sequential_time = seq_total_times[0]  # Sequential time is the first entry

    # Read total times for static and dynamic parallelism
    stat_num_workers, stat_total_times = read_times(path + f'/results/{country}/time_stat_{size}.txt')
    dyn_num_workers, dyn_total_times = read_times(path + f'/results/{country}/time_dyn_{size}.txt')
    ff_num_workers, ff_total_times = read_times(path + f'/results/{country}/time_ff_{size}.txt')

    # Calculate speedup for static and dynamic parallelism
    stat_speedup = [sequential_time / time for time in stat_total_times]
    dyn_speedup = [sequential_time / time for time in dyn_total_times]
    ff_speedup = [sequential_time / time for time in ff_total_times]

    y = [1 / (f + (1 - f) / worker) for worker in stat_num_workers]

    # Plot speedup versus number of workers for static and dynamic parallelism
    plt.plot(stat_num_workers, stat_speedup, marker='o', label='Static Parallelism')
    plt.plot(dyn_num_workers, dyn_speedup, marker='s', label='Dynamic Parallelism')
    plt.plot(ff_num_workers, ff_speedup, marker='s', label='FastFlow Parallelism')
    
    # Plot the diagonal
    max_workers = max(max(stat_num_workers), max(dyn_num_workers), max(ff_num_workers))
    plt.plot(range(max_workers + 1), range(max_workers + 1), linestyle='--', color='r', label='Ideal Speedup (x=y)')
    plt.plot(stat_num_workers, y, linestyle='--', color='g', label='Amdahl\'s Ideal Speedup')
    
    
    plt.xlabel('Number of Workers')
    plt.ylabel('Speedup')
    plt.title(f'Speedup vs. Number of Workers - {country.capitalize()} ({size})')
    plt.legend()
    
    # Customize ticks on x-axis
    plt.xticks(stat_num_workers + [max_workers], labels=[str(worker) if worker != max_workers else f'{worker}\n(Hyperthreading)' for worker in stat_num_workers + [max_workers]])
    plt.tight_layout()
    
    # Remove the grid
    plt.grid(False)

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/speedup_plot_{country}_{size}.png')

    # Show the plot
    plt.show()

def plot_scalability(country, size):
    # Scalability is defined as parallel time with 1 worker / parallel time with n workers
    # Read total times for static and dynamic parallelism
    stat_num_workers, stat_total_times = read_times(path + f'/results/{country}/time_stat_{size}.txt')
    dyn_num_workers, dyn_total_times = read_times(path + f'/results/{country}/time_dyn_{size}.txt')
    ff_num_workers, ff_total_times = read_times(path + f'/results/{country}/time_ff_{size}.txt')

    # Calculate scalability for static and dynamic parallelism
    stat_scalability = [stat_total_times[0] / time for time in stat_total_times]
    dyn_scalability = [dyn_total_times[0] / time for time in dyn_total_times]
    ff_scalability = [ff_total_times[0] / time for time in ff_total_times]


    # Plot scalability versus number of workers for static and dynamic parallelism
    plt.plot(stat_num_workers, stat_scalability, marker='o', label='Static Parallelism')
    plt.plot(dyn_num_workers, dyn_scalability, marker='s', label='Dynamic Parallelism')
    plt.plot(ff_num_workers, ff_scalability, marker='s', label='FastFlow Parallelism')

    max_workers = max(max(stat_num_workers), max(dyn_num_workers), max(ff_num_workers))
    plt.plot(range(max_workers + 1), range(max_workers + 1), linestyle='--', color='r', label='Ideal Speedup (x=y)')

    plt.xlabel('Number of Workers')
    plt.ylabel('Scalability')
    plt.title(f'Scalability vs. Number of Workers - {country.capitalize()} ({size})')
    plt.legend()

    # Customize ticks on x-axis
    plt.xticks(stat_num_workers + [max_workers], labels=[str(worker) if worker != max_workers else f'{worker}\n(Hyperthreading)' for worker in stat_num_workers + [max_workers]])
    
    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    plt.tight_layout()

    # Save the plot as an image
    plt.savefig(path + f'/images/scalability_plot_{country}_{size}.png')

    # Show the plot
    plt.show()

def plot_efficiency(country, size):
    # Efficiency is defined as speedup / number of workers
    # Read total times for static and dynamic parallelism
    stat_num_workers, stat_total_times = read_times(path + f'/results/{country}/time_stat_{size}.txt')
    dyn_num_workers, dyn_total_times = read_times(path + f'/results/{country}/time_dyn_{size}.txt')
    ff_num_workers, ff_total_times = read_times(path + f'/results/{country}/time_ff_{size}.txt')
    seq_num_workers, seq_total_times = read_times(path + f'/results/{country}/time_seq_{size}.txt')
    sequential_time = seq_total_times[0]  # Sequential time is the first entry

    # Calculate speedup for static and dynamic parallelism
    stat_speedup = [sequential_time / time for time in stat_total_times]
    dyn_speedup = [sequential_time / time for time in dyn_total_times]
    ff_speedup = [sequential_time / time for time in ff_total_times]

    # Calculate efficiency for static and dynamic parallelism
    stat_efficiency = [speedup / workers for speedup, workers in zip(stat_speedup, stat_num_workers)]
    dyn_efficiency = [speedup / workers for speedup, workers in zip(dyn_speedup, dyn_num_workers)]
    ff_efficiency = [speedup / workers for speedup, workers in zip(ff_speedup, ff_num_workers)]

    max_workers = max(max(stat_num_workers), max(dyn_num_workers), max(ff_num_workers))

    # Plot efficiency versus number of workers for static and dynamic parallelism
    plt.plot(stat_num_workers, stat_efficiency, marker='o', label='Static Parallelism')
    plt.plot(dyn_num_workers, dyn_efficiency, marker='s', label='Dynamic Parallelism')
    plt.plot(ff_num_workers, ff_efficiency, marker='s', label='FastFlow Parallelism')

    # Plot ideal efficiency of 1
    plt.plot(stat_num_workers, [1] * len(stat_num_workers), linestyle='--', color='r', label='Ideal Efficiency (y=1)')

    plt.xlabel('Number of Workers')
    plt.ylabel('Efficiency')
    plt.title(f'Efficiency vs. Number of Workers - {country.capitalize()} ({size})')
    plt.legend()

    # Customize ticks on x-axis
    plt.xscale('log')
    plt.xticks(stat_num_workers + [max_workers], labels=[str(worker) if worker != max_workers else f'{worker}\n(Hyperthreading)' for worker in stat_num_workers + [max_workers]])
    plt.tight_layout()

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/efficiency_plot_{country}_{size}.png')

    # Show the plot
    plt.show()

def plot_serial_fraction_dyn(country, size):
    # Plot the serial fraction for dynamic parallelism
    # Plot a bar plot where you put in percentage the serial time and the parallel time
    # So you have all bars of height 100% and you split them in two: one for serial and one for parallel
    
    # Read serial fraction for dynamic parallelism
    dyn_num_workers, dyn_serial_fraction = read_serial_fraction(path + f'/results/{country}/time_dyn_{size}.txt')

    # Plot the percentage of serial and parallel time for dynamic parallelism
    plt.bar(range(len(dyn_num_workers)), dyn_serial_fraction, width=0.4, align='center', label='Serial Time')
    plt.bar(range(len(dyn_num_workers)), [100 - serial for serial in dyn_serial_fraction], width=0.4, align='center', bottom=dyn_serial_fraction, label='Parallel Time')

    # Write on the bars the value
    for i, serial in enumerate(dyn_serial_fraction):
        plt.text(i, serial / 2, f'{serial:.2f}%', ha='center', va='center', color='white')
        plt.text(i, 100 - (100 - serial) / 2, f'{100 - serial:.2f}%', ha='center', va='center', color='black')
    # Plot the line

    plt.xlabel('Number of Workers')
    plt.ylabel('Percentage of Time')
    plt.title(f'Serial Fraction vs. Number of Workers - {country.capitalize()} ({size})')
    plt.legend()

    # Customize ticks on x-axis
    plt.xticks(range(len(dyn_num_workers)), labels=dyn_num_workers)
    plt.tight_layout()

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/serial_fraction_dyn_plot_{country}_{size}.png')

    # Show the plot
    plt.show()
    
def plot_overhead_decomposition(country, size, num_worker, n_gen=10):
    # Read overhead decomposition for static and dynamic parallelism
    stat_overheads = read_overhead(path + f'/results/{country}/time_stat_{size}.txt', num_worker, n_gen)
    dyn_overheads = read_overhead(path + f'/results/{country}/time_dyn_{size}.txt', num_worker, n_gen)

    # Calculate the ideal time
    seq_num_workers, seq_total_times = read_times(path + f'/results/{country}/time_seq_{size}.txt')
    sequential_time = seq_total_times[0]  # Sequential time is the first entry
    ideal_time = sequential_time / num_worker

    # Calculate the overhead decomposition for static and dynamic parallelism
    # Whole bar, overhead part, imbalance part, serial time part, other part
    stat_overhead_decomposition = [stat_overheads[0] - ideal_time, stat_overheads[1], stat_overheads[2], stat_overheads[3], stat_overheads[0] - stat_overheads[1] - stat_overheads[2] - stat_overheads[3]]
    dyn_overhead_decomposition = [dyn_overheads[0] - ideal_time, dyn_overheads[1], dyn_overheads[2], dyn_overheads[3], dyn_overheads[0] - dyn_overheads[1] - dyn_overheads[2] - dyn_overheads[3]]

    # Define colors slightly darker
    colors = ['#6BB8E6', '#CD5C5C', '#3CB371', '#FFD700']  # Lighter Blue, Lighter Red, Lighter Green, Gold

    # Plot the overhead decomposition in a bar of total size stat_overheads_decomposition[0], divided into the other components
    plt.bar(0, stat_overhead_decomposition[4], width=0.4, align='center', color=colors[0])  # Lighter Blue
    plt.bar(0, stat_overhead_decomposition[3], width=0.4, align='center', color=colors[1], bottom=stat_overhead_decomposition[4])  # Lighter Red
    plt.bar(0, stat_overhead_decomposition[2], width=0.4, align='center', color=colors[2], bottom=stat_overhead_decomposition[4] + stat_overhead_decomposition[3])  # Lighter Green
    plt.bar(0, stat_overhead_decomposition[1], width=0.4, align='center', color=colors[3], bottom=stat_overhead_decomposition[4] + stat_overhead_decomposition[3] + stat_overhead_decomposition[2])  # Gold

    plt.bar(1, dyn_overhead_decomposition[4], width=0.4, align='center', color=colors[0])  # Lighter Blue
    plt.bar(1, dyn_overhead_decomposition[3], width=0.4, align='center', color=colors[1], bottom=dyn_overhead_decomposition[4])  # Lighter Red
    plt.bar(1, dyn_overhead_decomposition[2], width=0.4, align='center', color=colors[2], bottom=dyn_overhead_decomposition[4] + dyn_overhead_decomposition[3])  # Lighter Green
    plt.bar(1, dyn_overhead_decomposition[1], width=0.4, align='center', color=colors[3], bottom=dyn_overhead_decomposition[4] + dyn_overhead_decomposition[3] + dyn_overhead_decomposition[2])  # Gold

    # Manually create legend without duplicates
    plt.legend(handles=[
        Rectangle((0, 0), 1, 1, color=colors[0], label='Other'),
        Rectangle((0, 0), 1, 1, color=colors[1], label='Serial Time'),
        Rectangle((0, 0), 1, 1, color=colors[2], label='Imbalance'),
        Rectangle((0, 0), 1, 1, color=colors[3], label='Overhead')
    ])

    plt.xlabel('Parallelism Type')
    plt.ylabel('Time (usecs)')
    plt.title(f'Difference from Ideal Time Decomposition - {country.capitalize()} ({size})')

    # Customize ticks on x-axis
    plt.xticks([0, 1], labels=['Static \nParallelism', 'Dynamic \nParallelism'])
    plt.tight_layout()

    # Create 'images' folder if it doesn't exist
    if not os.path.exists(path + '/images'):
        os.makedirs(path + '/images')

    # Save the plot as an image
    plt.savefig(path + f'/images/overhead_decomposition_plot_{country}_{size}.png')

    # Show the plot
    plt.show()


path = os.getcwd()
countries = ['canada']
sizes = ['1024', '4096']

for country in countries:
    for size in sizes:
        plot_speedup(country, size)
        plot_load_imbalance(country, size, max_y = 20)
        plot_scalability(country, size)
        plot_efficiency(country, size)
        #plot_serial_fraction_dyn(country, size)
        plot_overhead_decomposition(country, size, num_worker=16)


"""
text1 = "----- Recording run times -----
Number of workers: 1
Initialization time: 342945
Selection average time: 4781 +- 2393.46
Crossover average time: 8.44756e+06 +- 84508.3
Mutation average time: 105 +- 133.578
Evaluation average time: 49884 +- 11428.6
Merge average time: 14845 +- 7745.5

Total time: 85519984

text2 = ----- Recording run times -----
Number of workers: 1
Initialization time: 1312363
Selection average time: 33926 +- 8742.2
Crossover average time: 3.35529e+07 +- 220895
Mutation average time: 2573 +- 7240.63
Evaluation average time: 209981 +- 71247.9
Merge average time: 62353 +- 54699.7

Total time: 339954719

"""
def calculate_serial_time():
    #calculate the serial time of the text above

    a = (342945 + 14845 * 10 ) / (342945 + 10* (4781 + 84475600 + 105 + 49884 + 14845)) * 100
    b = (1312363 + 62353 * 10)  / (1312363 + 10* (33926 + 339954719 + 2573 + 209981 + 62353)) * 100
    return a, b

print(calculate_serial_time())

def plot_law():
    # Plot this law: 1 / (f + (1-f)/x) where f is the serial fraction and x is the number of workers
    # Take f = 0.5 and x = 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536
    f = 0.00057
    x = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
    y = [1 / (f + (1 - f) / worker) for worker in x]

    plt.plot(x, y, linestyle='--', color='g', label='Amdahl\'s Ideal Speedup')
    plt.xlabel('Number of Workers')
    plt.ylabel('Speedup')
    plt.title('Amdahl\'s Law')
    # SHow just the two last ticks, all the first ones are too close
    plt.xticks(x[-2:], labels=x[-2:])
    plt.savefig(path + '/images/amdahl_law.png')
    plt.show()
plot_law()


