import csv
import matplotlib.pyplot as plt
import glob

colNo = 5
sy = []

infile = "./response_times.csv"
x = []
y = []

fig = plt.figure()
ax = fig.add_subplot(111)
with open(infile, 'rb') as csvfile:
    reader = csv.reader(csvfile)
    for i, row in enumerate(csv.reader(csvfile)):
        if not 'nan' in row[colNo]:
            x.append(i)
            y.append(int(row[colNo]))

ax.set_ylim(0, 1.15*max(y))
ax.set_title(infile)
ax.plot(x, y)
plt.show()
