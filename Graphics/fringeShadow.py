import matplotlib.pyplot as plt
import matplotlib.patches as patches

fig1 = plt.figure()
ax1 = fig1.add_subplot(111,aspect='equal')

# Regions
ax1.add_patch( patches.Rectangle((.40, .3), .1, .4, linestyle='dashed',
fill=False) )
ax1.add_patch( patches.Rectangle((.45, .3), .05, .4, linestyle='dashed',
fill=False) )
ax1.add_patch( patches.Rectangle((.1, .3), .4, .4, fill=False, linewidth=2) )
ax1.add_patch( patches.Rectangle((.5, .3), .4, .4, fill=False, linewidth=2) )

# Region Labels
ax1.text(.11, .65, 'A', fontsize=15)
ax1.text(.51, .65, 'B', fontsize=15)
ax1.text(.41, .65, 'F', fontsize=15)
ax1.text(.46, .65, 'S', fontsize=15)

# Subscripts to labels (left out for now)
"""
ax1.text(.42, .65, '$ _B$', fontsize=10)
ax1.text(.475, .65, '$ _B$', fontsize=10)
"""

# Epsilons
ax1.text(.45, .25, '{', rotation=90, fontsize=15)
ax1.text(.40, .25, '{', rotation=90, fontsize=15)
ax1.text(.46, .24, '$\epsilon$', fontsize=15)
ax1.text(.41, .24, '$\epsilon$', fontsize=15)

fig1.savefig('Fringe&Shadow.png')
