#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np

for i in range(4):
    f = open('snapshot' + str(i + 1))
    lat = []
    lon = []
    cl = []
    for line in f:
        coords = line[1 + line.find(":"):].replace('\n','').split(',')
        if float(coords[2]) != 0.0:
            lon.append(float(coords[0]))
            lat.append(float(coords[1]))
            cl.append(float(coords[2]))
    m = min(cl)
    M = max(cl)
    r = M - m
    if r != 0:
        cl = (np.array(cl) - m)/r
    f.close()
    print cl[0] 
    plt.scatter(lon, lat, c=cl, linewidth='0', s=60)
    plt.xlabel("longitude")
    plt.ylabel("latitude")
    plt.show()
