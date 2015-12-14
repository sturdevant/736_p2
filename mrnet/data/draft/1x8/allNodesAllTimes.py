import glob
import matplotlib.pyplot as plt
import numpy as np
times = []
adds = []
points = []
s = set()

for fname in glob.glob('./LN_stats_*'):
    s.add(int(fname[fname.rfind("_")+1:]))
i = 0
for time in sorted(s):
    times.append(i)
    i += 1
    add = 0
    point = 0
    for fname in glob.glob('./LN_stats_*_' + str(time)):
        f = open(fname)
        for line in f:
            if "Adds" in line:
                add += int(line[line.rfind(":") + 1:])
            if "Points" in line:
                point += int(line[line.rfind(":") + 1:])
        f.close()
    adds.append(add)
    points.append(point)

old2 = 0
old = 0
tmp = []
for add in adds:
    new = add
    tmp.append((old2 + old + new)/3)
    old2 = old
    old = new
adds = tmp
tmp = []
old2 = 0
old = 0
for time in times:
    new = time
    tmp.append((old2 + old + new)/3)
    old2 = old
    old = new
times = tmp

fig = plt.figure()
ax = plt.gca()
plt.title('adds')
ax.scatter(times, adds)
plt.show()
