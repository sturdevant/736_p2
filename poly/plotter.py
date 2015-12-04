import matplotlib.pyplot as plt
import numpy as np

f = open('../snapshot')
x = []
y = []
for line in f:
    coords = line[1 + line.find(":"):].replace('\n','').split(',')
    x.append(coords[0])
    y.append(coords[1])
f.close()
plt.scatter(x, y)
plt.xlabel("x")
plt.ylabel("y")

plt.show()
