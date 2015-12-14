#!/usr/bin/env python
import glob
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

eps = 1
c = {'0':'#FFAA99', '1':'#66FFCC', '2':'#66CCFF', '3':'#DDDDDD', '4': 'blue', '5': 'red', '6':'green', '7':'yellow', '8':'orange', '9': 'red', '10': 'pink', '11':'purple', '12': 'blue', '13':'green', '14':'yellow', '15':'orange'}

s = set()
print "going through directory to get timestamps"
for f in glob.glob('./LN_points_*'):
    index = f.rfind('_')
    s.add(f[index:])

print "got all timestamps"

for t in s:
    fig = plt.figure()
    ax = fig.add_subplot(111)

    # Go through assignments to print grid
    print "getting assignments for grid at time " + t
    
    for fname in glob.glob('./LN_assigns_*' + t):
        i = fname[13]
        f = open(fname)
        for line in f:
            arr = line.rstrip().split(',')
            if arr[2] == '1':
                x = float(arr[1])
                y = float(arr[0])
                ax.add_patch(patches.Rectangle((x,y), eps, eps, linestyle='dotted', fill = False, linewidth = .5, color = '#FFFFFF'))
     
    print "got assignment grid" 
    lat = []
    lon = []
    cl = []
    ulat = []
    ulon = []
    hasCluster = False
    print "going through all files to get points/clusters"
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
    print "got all clusters"
    if hasCluster:
        m = min(cl)
        M = max(cl)
        r = M - m
        if r != 0:
            cl = (np.array(cl) - m)/r
    print "Printing ulon & ulat"
    print ulon, ulat
    #ax.scatter(ulon, ulat, c='#DDDDDD', linewidth='0', s=5)
    #if hasCluster:
        #ax.scatter(lon, lat, c='#000000', linewidth='0', s=5)

    plt.xlabel("longitude")
    plt.ylabel("latitude")
    ax.set_xlim([-126.848974, -64.885444])
    ax.set_ylim([22.396308, 51.384358])
    print "saving plot"
    plt.savefig('../images/plot' + t + '.png')
