import glob
import matplotlib.pyplot as plt
nodeNum = 0
times = []
adds = []
points = []

# All the values we're interested in
filtered = 0
shadowUpdates = 0
replacements = 0
adds = 0
queries = 0
duplicates = 0
dupClustered = 0
totalPoints = 0
totalClustered = 0
totalWritten = 0

a = {"Filtered":0, "Shadow Updates":0, "Replacements":0, "Adds":0, "Queries":0,
"Duplicates:":0, "Duplicates (clustered)":0, "Total Points:":0, "Total Points
(clustered)":0, "Total Points (written)":0}

nFiles = 0
for fname in glob.glob('./LN_stats_' + str(nodeNum) + '_*'):
   nFiles += 1
   f = open(fname)
   for line in f:
        for name in a.iterkeys():
            if name in line:
                a[name] += int(line[line.rfind(":") + 1:])          
   f.close()
print float(a["Filtered"])/nFiles
