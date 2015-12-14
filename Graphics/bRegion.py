import matplotlib.pyplot as plt
import matplotlib.lines as lines
import matplotlib.patches as patches
import matplotlib.transforms as transforms

fig1 = plt.figure()
ax1 = fig1.add_subplot(111, aspect='equal')

ax1.add_patch( patches.Rectangle((.1, .1), .8, .7, fill=False, linewidth=3))
ax1.add_patch( patches.Rectangle((.1 + .8/3, .1), 2*.8/3, .7, lw=2, fill=False))
ax1.add_patch( patches.Rectangle((.1 + 2*.8/3, .1), .8/3, .7, lw=2, fill=False))
fig1.savefig('bRegion.png')
