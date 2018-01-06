#!/usr/bin/env python3

import argparse
from datetime import datetime
import matplotlib.pyplot as plt 

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
        To.append(float(parameters[2]))
        Ts.append(float(parameters[3]))
        Th.append(float(parameters[4]))
        iTerm.append(float(parameters[5]))
        kP.append(float(parameters[6]))
        kI.append(float(parameters[7]))
        kD.append(float(parameters[8]))
        flame.append(1 if int(parameters[9], 0) & 0x08 else 0)
        chEnable.append(1 if int(parameters[9], 0) & 0x02 else 0)

plt.plot(timestamp, Ti)
plt.plot(timestamp, To)
plt.plot(timestamp, Ts)
plt.plot(timestamp, Th)

plt.show()