import matplotlib.pyplot as plt
import numpy as np

f = open('node_assigns_2')
lat = []
lon = []
for line in f:
    coords = line.replace('\n','').split(',')
    if int(coords[2]) == 1:
        lat.append(float(coords[0]))
        lon.append(float(coords[1]))
ax = plt.subplot(111)
ax.set_xlim([-1.5, 1.5])
ax.set_ylim([-1.5, 1.5])
f.close()
ax.scatter(lon, lat)
plt.savefig('data2.png')
plt.clf()
