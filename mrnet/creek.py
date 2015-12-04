#!/usr/bin/env python

import random
random.seed()

def r(m, M):
   return random.random()*(M-m) + m
nPoints = 20
xMin = -180.0
xMax = 180.0
yMin = -90.0
yMax = 90.0

for i in range(nPoints):
   x = r(xMin, xMax)
   y = r(yMin, yMax)
   print "t" + str(i) + "\n" + str(x) + "," + str(y)
