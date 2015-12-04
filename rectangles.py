import matplotlib.pyplot as plt
import matplotlib.patches as patches
colors = {'0':'#FFFFFF', '1':'#66FFCC', '2':'#66CCFF', '3':'#DDDDDD'}
eps = 1.25
fig1 = plt.figure()
ax1 = fig1.add_subplot(111,aspect='equal')
ax1.set_xlim([-10, 10])
ax1.set_ylim([-10, 10])
f = open('assigns_combined')
for line in f:
   arr = line.rstrip().split(',')
   x = float(arr[0])
   y = float(arr[1])
   col = colors[arr[2]]
   if (col != "#FFFFFF"):
      ax1.add_patch( patches.Rectangle((x, y), eps, eps, facecolor=col) )
fig1.savefig('rectangle.png')
