import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

fig1 = plt.figure()
ax1 = fig1.add_subplot(111,aspect='equal')

# Coordinates for p & q
pathxs = np.array([.2, .3, .4, .5, .6, .7, .8])
pathys = np.array([.2, .3, .4, .6, .6, .7, .8])
pathdx = pathxs[1:] - pathxs[:-1]
pathdy = pathys[1:] - pathys[:-1]
print pathdx
print pathdy

xs = pathxs
ys = pathys

# Draw points p & q with labels offset by .01 in x
for x, y, dx, dy in zip(pathxs[:-1], pathys[:-1], pathdx, pathdy):
    plt.arrow(x, y, dx, dy, length_includes_head=True)
plt.scatter(xs, ys, alpha=1, color='green', s=80)

fig1.savefig('pathing.png')
