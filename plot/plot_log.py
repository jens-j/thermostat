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
flame = []
chEnable = []

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
        flame.append(1 if int(parameters[9], 0) & 0x08 else 0)
        chEnable.append(1 if int(parameters[9], 0) & 0x02 else 0)

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
plt.plot(timestamp, flame, label='flame on')
plt.plot(timestamp, np.array(chEnable) + 0.01, label='CH enabled')
plt.legend(loc='upper left')

# plt.subplot(4, 1, 4)
# plt.plot(timestamp, Ti, label='room temperature')
# plt.plot(timestamp, Ts, label='room setpoint')
# plt.plot(timestamp, To, label='heater setpoint')
# plt.plot(timestamp, iTerm, label='error sum')
# plt.legend(loc='upper right')

plt.show()