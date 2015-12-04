import matplotlib.pyplot as plt
import matplotlib.patches as patches
colors = {'0':'#FFFFFF', '1':'#66FFCC', '2':'#66CCFF', '3':'#DDDDDD',
'4':'#FFFFFF'}
eps = 1.0
fig1 = plt.figure()
ax1 = fig1.add_subplot(111,aspect='equal')
ax1.set_ylim([24.4, 49.4])
ax1.set_xlim([-124.9, -66.9])
f = open('iaall')
for line in f:
   arr = line.rstrip().split(',')
   y = float(arr[0])
   x = float(arr[1])
   col = colors[arr[2]]
   if (col != "#FFFFFF"):
      ax1.add_patch( patches.Rectangle((x, y), eps, eps, facecolor=col) )
fig1.savefig('rectangle.png')
