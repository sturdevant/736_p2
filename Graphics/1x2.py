import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

fig1 = plt.figure()
ax1 = fig1.add_subplot(111, aspect='equal')

x = [.25 + .05*np.cos(np.pi/4), .5 - .05*np.cos(np.pi/4)]
y = [.25 + .05*np.sin(np.pi/4), .5 - .05*np.sin(np.pi/4)]
plt.plot(x, y, color='black')

x = [.5 + .05*np.cos(np.pi/4), .75 - .05*np.cos(np.pi/4)]
y = [.5 - .05*np.cos(np.pi/4), .25 + .05*np.cos(np.pi/4)]
plt.plot(x, y, color='black')

ax1.add_patch( patches.Ellipse((.5, .5), .1, .1, fc="white"))
ax1.add_patch( patches.Ellipse((.25, .25), .1, .1, fc="white"))
ax1.add_patch( patches.Ellipse((.75, .25), .1, .1, fc="white"))

ax1.text(.485, .485, 'A', fontsize=15)
ax1.text(.235, .235, 'B', fontsize=15)
ax1.text(.735, .235, 'C', fontsize=15)

ax1.set_xlim([0, 1])
ax1.set_ylim([0, 1])

fig1.savefig('1x2.png')
