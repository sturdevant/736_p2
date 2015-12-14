import glob
import argparse
import numpy as np
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()
parser.add_argument('-n', nargs="+")
parser.add_argument('-p', action="store_true")
args = parser.parse_args()

# All the values we're interested in
a = {"Filtered:":{}, "Shadow Updates:":{}, "Replacements:":{}, "Adds:":{}, "Queries:":{}, "Duplicates:":{}, "Duplicates (clustered):":{}, "Total Points:":{}, "Total Points (clustered):":{}, "Total Points (written):":{}}
nFiles = 0
times = set()
for fname in glob.glob('./testing/results/LN_stats_*'):
   times.add(int(fname[fname.rfind('_') + 1:]))
for nodeNum in args.n:
    for name in a.iterkeys():
        a[name][nodeNum] = []
    for t in times:
        for fname in glob.glob('./testing/results/LN_stats_' + str(nodeNum) + '_' + str(t)):
           nFiles += 1
           f = open(fname)
           for line in f:
                for name in a.iterkeys():
                    if name in line:
                        a[name][nodeNum].append(int(line[line.rfind(":") + 1:]))
           f.close()

print "Sums:"
for nodeNum in args.n:
    print '\tNode ' + nodeNum + ":"
    for key in a.iterkeys():
        print '\t\t' + key,
        l = len(key)
        while l < 30:
            print "",
            l += 1
        print str(sum(a[key][str(nodeNum)]))

print "Averages:"
for nodeNum in args.n:
    print '\tNode ' + nodeNum + ":"
    for key in a.iterkeys():
        print '\t\t' + key,
        l = len(key)
        while l < 30:
            print "",
            l += 1
        print str(float(sum(a[key][nodeNum]))/len(a[key][nodeNum]))

if args.p:
    for nodeNum in args.n:
        for key in a.iterkeys():
            ts = np.array(list(times))
            vs = np.array(a[key][nodeNum])
            smoothT = (ts[1:] + ts[:-1])/2
            smoothV = (vs[1:] + vs[:-1])/2

            plt.title("Node " + nodeNum + " " + key[:-1] + " Unsmooth")
            plt.scatter(ts, vs)
            plt.show()
            plt.title("Node " + nodeNum + " " + key[:-1] + " Smooth")
            plt.scatter(smoothT, smoothV) 
            plt.show()
