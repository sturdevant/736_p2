import numpy as np
import matplotlib.pyplot as plt

fig1 = plt.figure()
ax1 = fig1.add_subplot(111, aspect='equal')

m = -1
x = np.linspace(0, 1)
yInts = np.linspace(-1, 2)

for b in yInts:
   plt.plot(x, m*x + b, color='black', lw = 3)

ax1.set_xlim([0, 1])
ax1.set_ylim([0, 1])

fig1.savefig('hatch.png')
