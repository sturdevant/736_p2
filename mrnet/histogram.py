#!/usr/bin/env python

import matplotlib.pyplot as plt

# Read in times from input file
times = []
f = open('response_times')
for line in f:
   val = line.split(',')[1]
   print val
   print val.isspace()
   if not "nan" in val and not val == "" and not val.isspace():
       times.append(float(val))
f.close()

n, bins, patches = plt.hist(times, 5)
plt.plot(bins)
plt.show()
