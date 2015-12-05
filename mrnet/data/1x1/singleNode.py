import glob
import matplotlib.pyplot as plt
nodeNum = 0
times = []
adds = []
points = []

filtered = 0
nFiles = 0
for fname in glob.glob('./LN_stats_' + str(nodeNum) + '_*'):
   nFiles += 1
   f = open(fname)
   for line in f:
      if "Filtered" in line:
         filtered += int(line[line.rfind(":") + 1:])
   f.close()
print float(filtered)/nFiles
