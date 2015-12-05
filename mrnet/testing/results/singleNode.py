import glob
import matplotlib.pyplot as plt
nodeNum = 8
times = []
adds = []
points = []

for fname in glob.glob('./LN_stats_' + str(nodeNum) + '_*'):
   time = int(fname[fname.rfind("_")+1:])
   times.append(time)
   f = open(fname)
   for line in f:
      if "Adds" in line:
         adds.append(int(line[line.rfind(":") + 1:]))
      if "Points" in line:
         points.append(int(line[line.rfind(":") + 1:]))
   f.close()
plt.scatter(times, points)
plt.show()
