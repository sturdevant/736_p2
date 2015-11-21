/*
 * This file provides implementations for an internal node (non-leaf). The
 * public functions of internal nodes are described first.
 */

#include "ReturnCode.h"
#include "Request.h"
#include "Response.h"
#include "Point.h"
#include "Cell.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <queue>

class LeafNode {
public:
   LeafNode(unsigned int nDims,
            double epsilon,
            unsigned int uniqueId,
            double ptRemovalThresh,
            double* mins,
            double* maxes,
            double* assignedMins,
            double* assignedMaxes,
            ReturnCode (*requestFunc)(unsigned int, Request*),
            ReturnCode (*responseFunc)(Response*));

   ~LeafNode();
   ReturnCode query(Request* req, Response* res);
   void snapshot(FILE* ptFile, FILE* assignmentFile);

private:
   ReturnCode handleUpdate(Request* req, Response* res);
   ReturnCode handleAdd(Request* req, Response* res);
   ReturnCode handlePoint(Request* req, Response* res);
   ReturnCode handlePolygon(Request* req, Response* res);
   unsigned long getNewClusterId();
   void makeCluster(Point* pt);
   void addToCluster(Cluster* clust, Point* pt);
   Cluster* mergeClusters(std::vector<Cluster*> clusters, Point* pt);
   void removeFromCluster(Cluster* clust, Point* pt);
   bool verifyCluster(Cluster* clust, Point* pt);
   double countNeighbors(Point* pt, unsigned long time);
   double addToNeighbors(Point* pt, unsigned long time);
   void replaceClusterFromPoint(Cluster* oldClust, Cluster* newClust, Point* pt);
   std::vector<Cluster*> getNeighborClusters(Point* pt, unsigned long time);
   void getPointNeighborList(Point* point, std::vector<std::vector<Point*>*>& neighborList);
   void getPointOwnerGroup(Point* point, std::vector<unsigned int>& uniqueOwners);
   unsigned int getCellIndex(unsigned int* cell);
   Cell* getCell(unsigned int* cell);
   void getCellGroupRecursive(unsigned int* whichDims, 
                              unsigned int* cell,
                              std::vector<Cell*>& result,
                              unsigned int nDims);
   void getIndexGroupRecursive(unsigned int* cell,
                               unsigned int nDims,
                               std::vector<unsigned int>& result);
   std::vector<unsigned int>getPointIndexGroup(Point* pt);
   std::vector<Cell*> getPointCellGroup(Point* point);
   void setLengths(double* maxes);
   void setDimFactors();
   void setMaxIndex();
   void assignPointCell(Point* pt);
   void recursiveAssignSubslice(double* low, double* high, double* cur, unsigned int nDims);
   void assignCells(double* assignedMins, double* assignedMaxes);
   Cell* getPointCell(Point* point);
   void indexToDArray(unsigned int index, double* d);

   unsigned int dims;
   unsigned int uniqueId;
   double* mins;
   double* maxes;
   unsigned int* dimFactors;
   unsigned int maxIndex;
   unsigned int* lengths;
   ReturnCode (*requestFunc)(unsigned int, Request*);
   ReturnCode (*responseFunc)(Response*);
   Cell** cells;
   double eps, epsSqr;
   double ptRemovalThreshold;
};
