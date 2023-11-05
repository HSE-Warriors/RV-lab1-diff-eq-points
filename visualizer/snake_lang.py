# # import matplotlib.pyplot as plt
# # import matplotlib.animation as animation
# # import numpy as np
# # import csv
# # import re
# # from celluloid import Camera
# #
# # """
# # Body   :     x              y           vx              vy
# # Cycle 1
# # Body 1 : 0.000000	0.000000	0.000000	0.000000
# # Body 2 : 2.000000	2.000000	0.000001	0.000001
# # Body 3 : 3.000000	3.000000	-0.000001	-0.000001
# #
# # Cycle 2
# # ...
# # """
# #
# # # header_pattern = re.compile(r'(\d+),(\d+),(-?\d+\.\d+),(-?\d+\.\d+),(-?\d+\.\d+),(-?\d+\.\d+)')
# # # data_pattern = re.compile(r'Body (\d+) : (\S+)\s+(\S+)\s+(\S+)\s+(\S+)')
# # # csv_data = []
# # #
# # # with open('intro/oneThreadExample-output', 'r') as input_file:
# # #     current_cycle = None
# # #     for line in input_file:
# # #         line = line.strip()
# # #         if line.startswith("Cycle"):
# # #             current_cycle = line.split()[-1]
# # #         else:
# # #             match_data = data_pattern.match(line)
# # #             if match_data:
# # #                 body_num, x, y, vx, vy = match_data.groups()
# # #                 csv_data.append([current_cycle, body_num, x, y, vx, vy])
# # #
# # # with open('visualizer/output.csv', 'w', newline='') as output_file:
# # #     csv_writer = csv.writer(output_file)
# # #     csv_writer.writerow(["Cycle", "Body", "x", "y", "vx", "vy"])
# # #     csv_writer.writerows(csv_data)
# # #
# # # print(f"CSV data saved to 'visualizer/output.csv'")
# #
# # """
# # Cycle,Body,x,y,vx,vy
# # 1,1,0.000000,0.000000,0.000000,0.000000
# # 1,2,2.000000,2.000000,0.000001,0.000001
# # 1,3,3.000000,3.000000,-0.000001,-0.000001
# # 2,1,0.000000,0.000000,0.000001,0.000001
# # 2,2,2.000000,2.000000,0.000002,0.000002
# # ...
# # """
# #
# # data = []
# #
# # with open('visualizer/output.csv', 'r') as csv_file:
# #     csv_reader = csv.DictReader(csv_file)
# #     for row in csv_reader:
# #         data.append(row)
# #
# # fig, ax = plt.subplots()
# # camera = Camera(fig)
# #
# # cycles = sorted(set([row['Cycle'] for row in data]))
# # colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
# #
# # iteration_count = 0
# #
# # for cycle in cycles:
# #     cycle_data = [row for row in data if row['Cycle'] == cycle]
# #     bodies = [int(row['Body']) for row in cycle_data]
# #     x_values = [float(row['x']) for row in cycle_data]
# #     y_values = [float(row['y']) for row in cycle_data]
# #
# #     for body, x, y, color in zip(bodies, x_values, y_values, colors):
# #         plt.scatter(x, y, color=color, label=f'Body {body}')
# #
# #     plt.title(f'Cycle {cycle}')
# #     plt.xlabel('x')
# #     plt.ylabel('y')
# #     plt.legend(loc='upper right')
# #
# #     camera.snap()
# #
# #     iteration_count += 1
# #     if iteration_count % 100 == 0:
# #         print(f'Processed {iteration_count} cycles of {len(cycles)}')
# #     # if iteration_count == 500:
# #     #     break
# #
# # print("Finished, now drawing...")
# # animation = camera.animate(interval=1000)
# # animation.save('visualizer/output_animation.gif', writer='pillow')
# # plt.show()
#
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
# import numpy as np
# import csv
# import re
# from celluloid import Camera
#
# """
# Body   :     x              y           vx              vy
# Cycle 1
# Body 1 : 0.000000	0.000000	0.000000	0.000000
# Body 2 : 2.000000	2.000000	0.000001	0.000001
# Body 3 : 3.000000	3.000000	-0.000001	-0.000001
#
# Cycle 2
# ...
# """
#
# # header_pattern = re.compile(r'(\d+),(\d+),(-?\d+\.\d+),(-?\d+\.\d+),(-?\d+\.\d+),(-?\d+\.\d+)')
# # data_pattern = re.compile(r'Body (\d+) : (\S+)\s+(\S+)\s+(\S+)\s+(\S+)')
# # csv_data = []
# #
# # with open('intro/oneThreadExample-output', 'r') as input_file:
# #     current_cycle = None
# #     for line in input_file:
# #         line = line.strip()
# #         if line.startswith("Cycle"):
# #             current_cycle = line.split()[-1]
# #         else:
# #             match_data = data_pattern.match(line)
# #             if match_data:
# #                 body_num, x, y, vx, vy = match_data.groups()
# #                 csv_data.append([current_cycle, body_num, x, y, vx, vy])
# #
# # with open('visualizer/output.csv', 'w', newline='') as output_file:
# #     csv_writer = csv.writer(output_file)
# #     csv_writer.writerow(["Cycle", "Body", "x", "y", "vx", "vy"])
# #     csv_writer.writerows(csv_data)
# #
# # print(f"CSV data saved to 'visualizer/output.csv'")
#
# """
# Cycle,Body,x,y,vx,vy
# 1,1,0.000000,0.000000,0.000000,0.000000
# 1,2,2.000000,2.000000,0.000001,0.000001
# 1,3,3.000000,3.000000,-0.000001,-0.000001
# 2,1,0.000000,0.000000,0.000001,0.000001
# 2,2,2.000000,2.000000,0.000002,0.000002
# ...
# """
#
# data = []
#
# with open('visualizer/output.csv', 'r') as csv_file:
#     csv_reader = csv.DictReader(csv_file)
#     for row in csv_reader:
#         data.append(row)
#
# fig, ax = plt.subplots()
# camera = Camera(fig)
#
# cycles = sorted(set([row['Cycle'] for row in data]))
# colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
#
# iteration_count = 0
#
#
# def plot_cycle_data(cycle_data):
#     bodies = [int(row['Body']) for row in cycle_data]
#     x_values = [float(row['x']) for row in cycle_data]
#     y_values = [float(row['y']) for row in cycle_data]
#
#     for body, x, y, color in zip(bodies, x_values, y_values, colors):
#         plt.scatter(x, y, color=color, label=f'Body {body}')
#
#     plt.title(f'Cycle {cycle}')
#     plt.xlabel('x')
#     plt.ylabel('y')
#     plt.legend(loc='upper right')
#     camera.snap()
#
#
# data_by_cycles = {}
#
# for cycle in cycles:
#     cycle_data = [row for row in data if row['Cycle'] == cycle]
#     data_by_cycles[cycle] = cycle_data
#     iteration_count += 1
#     if iteration_count % 100 == 0:
#         print(f'Processed {iteration_count} cycles of {len(cycles)}')
#     # if iteration_count == 500:
#     #     break
#
# for cycle in cycles:
#     plot_cycle_data(data_by_cycles[cycle])
#     iteration_count += 1
#     if iteration_count % 100 == 0:
#         print(f'Processed {iteration_count} cycles of {len(cycles)}')
#
# print("Finished, now drawing...")
# animation = camera.animate(interval=1000)
# animation.save('visualizer/output_animation.gif', writer='pillow')
# plt.show()
#
# # Now, you have a dictionary 'data_by_cycles' containing the data for each cycle.


import matplotlib.pyplot as plt
import numpy as np
import csv
from celluloid import Camera


data = []

with open('visualizer/output.csv', 'r') as csv_file:
    csv_reader = csv.DictReader(csv_file)
    for row in csv_reader:
        data.append(row)

fig, ax = plt.subplots()
camera = Camera(fig)

cycles = sorted(set([row['Cycle'] for row in data]))
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']

iteration_count = 0


def plot_cycle_data(cycle_data):
    bodies = [int(row['Body']) for row in cycle_data]
    x_values = [float(row['x']) for row in cycle_data]
    y_values = [float(row['y']) for row in cycle_data]

    for body, x, y, color in zip(bodies, x_values, y_values, colors):
        plt.scatter(x, y, color=color, label=f'Body {body}')

    plt.title(f'Cycle {cycle_data[0]["Cycle"]}')  # Use the first row's cycle number
    plt.xlabel('x')
    plt.ylabel('y')
    plt.legend(loc='upper right')
    camera.snap()


data_by_cycles = {}

for cycle in cycles:
    cycle_data = [row for row in data if row['Cycle'] == cycle]
    data_by_cycles[cycle] = cycle_data
    iteration_count += 1
    if iteration_count % 100 == 0:
        print(f'Processed {iteration_count} cycles of {len(cycles)}')

print("Finished, now drawing frames...")

# Save each frame as a PNG
for cycle in cycles:
    plot_cycle_data(data_by_cycles[cycle])
    plt.savefig(f'visualizer/frames/frame_{cycle}.png')  # Save each frame as a separate PNG
    plt.clf()

print("Frames saved, now creating animation...")

# Now, create an animation from the saved frames
animation = camera.animate(interval=1000)
animation.save('visualizer/output_animation.gif', writer='pillow')

print("Animation saved.")
plt.show()