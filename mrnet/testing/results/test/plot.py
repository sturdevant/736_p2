import csv
import numpy as np
import matplotlib.pyplot as plt
import glob

def smooth(x, y):
   x = (x[:-4] + x[1:-3] + x[2:-2] + x[3:-1] + x[4:])/5
   y = (y[:-4] + y[1:-3] + y[2:-2] + y[3:-1] + y[4:])/5
   return (x, y)

colNo = 3

d = {}
minLen = None
for infile in glob.glob("./LN_stats_*.csv"):
   x = []
   y = []
   d[infile] = (x, y)
   with open(infile, 'rb') as csvfile:
      reader = csv.reader(csvfile)
      for i, row in enumerate(csv.reader(csvfile)):
         x.append(i)
         y.append(int(row[colNo]))
   if minLen == None or len(x) < minLen:
      minLen = len(x)

xs = np.array(x[:minLen])
ys = np.zeros(minLen)
for infile in glob.glob("./LN_stats_*.csv"):
   fig = plt.figure()
   ax = fig.add_subplot(111)
   plt.title(infile)
   ax.set_ylim([0, max(y)])
   x, y = d[infile]
   ys = ys + np.array(y[:minLen])
   x = np.array(x)
   y = np.array(y)
   x, y = smooth(x, y)
   ax.plot(x, y)
   plt.show()
   
fig = plt.figure()
ax = fig.add_subplot(111)
plt.title("Sums")
ax.set_ylim([0, max(ys)])
ax.plot(xs, ys)
plt.show()
