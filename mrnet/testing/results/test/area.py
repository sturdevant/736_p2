import csv
import numpy as np
import matplotlib.pyplot as plt
import glob

i = 0
colors = ['#FFDD99', '#77FFCC', '#77CCFF']
def cycle(arr):
   return arr[i%len(arr)]

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
xs, ys = smooth(xs, ys)

fig = plt.figure()
ax = fig.add_subplot(111)
for infile in glob.glob("./LN_stats_*.csv"):
   plt.title(infile)
   ax.set_ylim([0, max(y)])
   x, y = d[infile]
   x = np.array(x[:minLen])
   y = np.array(y[:minLen])
   x, y = smooth(x, y)
   ax.fill_between(x, ys, y + ys, facecolor=cycle(colors))
   i += 1
   ax.plot(x, y + ys, color='black')
   ys += y
   
plt.title("Area")
ax.set_ylim([0, max(ys)])
plt.show()
