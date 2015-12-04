#!/usr/bin/env python
import glob
import matplotlib.pyplot as plt
import numpy as np

s = set()
for f in glob.glob('./LN_points_*'):
    index = f.rfind('_')
    s.add(f[index:])

for t in s:
    lat = []
    lon = []
    cl = []
    ulat = []
    ulon = []
    hasCluster = False
    for fname in glob.glob('./LN_points_*' + t):
        f = open(fname)
        for line in f:
            coords = line[1 + line.find(":"):].replace('\n','').split(',')
            if float(coords[2]) != 0.0:
                hasCluster = True 
                lat.append(float(coords[0]))
                lon.append(float(coords[1]))
                cl.append(float(coords[2]))
            else:
                ulat.append(float(coords[0]))
                ulon.append(float(coords[1]))
    f.close()
    if hasCluster:
        m = min(cl)
        M = max(cl)
        r = M - m
        if r != 0:
            cl = (np.array(cl) - m)/r
    plt.scatter(ulon, ulat, c='#DDDDDD', linewidth='0', s=5)
    if hasCluster:
        plt.scatter(lon, lat, c=cl, linewidth='0', s=5)

    plt.xlabel("longitude")
    plt.ylabel("latitude")
    plt.savefig('plot' + t + '.png')
