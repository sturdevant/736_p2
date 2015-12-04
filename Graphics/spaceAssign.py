import matplotlib.pyplot as plt
import matplotlib.patches as patches

fig1 = plt.figure()
ax1 = fig1.add_subplot(111,aspect='equal')

# Coordinates for p & q
xs = [.2, .8]
ys = [.2, .8]
ls = ['p','q']

# Draw regions
ax1.add_patch( patches.Rectangle((.1, .1), .4, .4, fill=False, linewidth=1) )
ax1.add_patch( patches.Rectangle((.5, .1), .4, .4, fill=False, linewidth=1) )
ax1.add_patch( patches.Rectangle((.1, .1), .4, .8, fill=False, linewidth=2) )
ax1.add_patch( patches.Rectangle((.5, .1), .4, .8, fill=False, linewidth=2) )

# Label regions
ax1.text(.11, .91, 'B', fontsize=15)
ax1.text(.51, .91, 'C', fontsize=15)
ax1.text(.11, .85, 'D', fontsize=15)
ax1.text(.51, .85, 'F', fontsize=15)
ax1.text(.11, .45, 'E', fontsize=15)
ax1.text(.51, .45, 'G', fontsize=15)
plt.title("A")

# Draw points p & q with labels offset by .01 in x
plt.scatter(xs, ys, color='black')
for x, y, l in zip(xs, ys, ls):
    ax1.text(x + .01, y, l, fontsize=10)

fig1.savefig('spaceAssign.png')
