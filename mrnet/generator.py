#!/usr/bin/env python

import random
random.seed()

def r(m, M):
   return random.random()*(M-m) + m
nPoints = 1000
xMin = -1.08
xMax = 1.05
yMin = -1.07
yMax = 1.06

for i in range(nPoints):
   x = r(xMin, xMax)
   y = r(yMin, yMax)
   print "t" + str(i) + "\n" + str(x) + "," + str(y)
