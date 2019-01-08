#!/usr/bin/env python3

import argparse
from datetime import datetime
import matplotlib.pyplot as plt 
import numpy as np

timestamp = []
Ti = []
To = []
Ts = []
Th = []
iTerm = []
kP = []
kI = []
kD = []
chEnable = []
dhwEnable = []
flame = []

parser = argparse.ArgumentParser()
parser.add_argument('filename', type=str, help='log filename')
args = parser.parse_args()

with open(args.filename) as f:
    
    for line in f:

        if line[0] == '#':
            continue

        parameters = line.strip().split(',');
        timestamp.append(datetime.strptime(parameters[0], '%d-%m-%y_%H:%M:%S'))
        Ti.append(float(parameters[1]))
        Ts.append(float(parameters[2]))
        To.append(float(parameters[3]))
        Th.append(float(parameters[4]))
        iTerm.append(float(parameters[5]))
        kP.append(float(parameters[6]))
        kI.append(float(parameters[7]))
        kD.append(float(parameters[8]))
        chEnable.append(1 if int(parameters[9], 0) & 0x02 else 0)
        dhwEnable.append(1 if int(parameters[9], 0) & 0x04 else 0)
        flame.append(1 if int(parameters[9], 0) & 0x08 else 0)

plt.figure(1)
plt.subplot(3, 1, 1)
plt.plot(timestamp, Ti, label='room temperature')
plt.plot(timestamp, Ts, label='room setpoint')
plt.legend(loc='upper left')

plt.subplot(3, 1, 2)
plt.plot(timestamp, Th, label='heater temperature')
plt.plot(timestamp, To, label='heater setpoint')
plt.legend(loc='upper left')

plt.subplot(3, 1, 3)
plt.plot(timestamp, flame, label='flame')
plt.plot(timestamp, np.array(chEnable) + 1.2, label='central heating')
plt.plot(timestamp, np.array(dhwEnable) + 2.4, label='domestic hot water')
plt.legend(loc='upper left')

# plt.subplot(4, 1, 4)
# plt.plot(timestamp, Ti, label='room temperature')
# plt.plot(timestamp, Ts, label='room setpoint')
# plt.plot(timestamp, To, label='heater setpoint')
# plt.plot(timestamp, iTerm, label='error sum')
# plt.legend(loc='upper right')

plt.show()