#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np

paramName = ["", "Mean Response Time", "Queries Sent", "Replays Sent", "Points added"]

f = open('response_times')

time = []
param = []

for i in range(0, 5):
    param.append([])

for line in f:
    coords = line.split(',')
    time.append(float(coords[0]))
    for i in range(1, 5):
        param[i].append(float(coords[i]))

f.close()

for i in range(1, 5):
    m = min(param[i])
    M = max(param[i])
    cl = []
    cl = (np.array(param[i]) - m)/(M - m)

    plt.scatter(time, param[i], c=cl, linewidth='1', s=60)
    plt.xlabel("Run time")
    plt.ylabel(paramName[i])
    plt.show()


