#ifndef _INTERNAL_NODE_H_
#define _INTERNAL_NODE_H_

#include <cmath>
//#include <ctgmath>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include "Request.h"
#include "Response.h"
#include "ReturnCode.h"
#include "Point.h"

typedef struct QIndex {
   unsigned int index;
   double dist;
} QIndex;

struct QIndexCompare : public std::binary_function<QIndex&, QIndex&, bool> {
   bool operator()(const QIndex& qi1, const QIndex& qi2) const {
      return qi1.dist > qi2.dist;
   }
};

class InternalNode {

public:

   InternalNode(unsigned int nDims,
                unsigned int nChildren,
                double epsilon,
                double minPts,
                double removalThreshold,
                double decayFactor,
                unsigned long decayRes,
                double* mins,
                double* maxes,
                double* aMins, 
                double* aMaxes,
                ReturnCode (*requestFunc)(unsigned int, Request*),
                ReturnCode (*responseFunc)(Response*),
                double* caMins,
                double* caMaxes);

   ~InternalNode();
   bool admitPoint(Point* pt);   
   ReturnCode addPoint(Request* req);
   ReturnCode query(Request* req);
   ReturnCode update(Response* res);
   void snapshot(FILE* assignmentFile);

private:
   
   unsigned int getPointOwner(double* pt);
   void getPointOwnerGroup(double* pt,
                           std::vector<unsigned int>& uniqueOwners);
   unsigned int getCellOwner(unsigned int* cell);
   void getCellOwnerGroup(unsigned int* whichDims,
                          unsigned int* cell,
                          std::vector<unsigned int>& uniqueOwners,
                          unsigned int nDims);
   void indexToDArray(unsigned int index, double* d);
   void setOwner(unsigned int* cell, unsigned int owner);
   void assignSubslice(unsigned int* whichDims,
                       unsigned int* cell,
                       unsigned int owner,
                       unsigned int nDims,
                       unsigned int* aLengths);
   void assignOwners(double* aMins, double* aMaxes, double* caMins, double* caMaxes);

   void setLengths();
   void setDimFactors();
   void setMaxIndex();

   unsigned int nChildren;
   unsigned int* owners;
   unsigned int* lengths;
   unsigned int* dimFactors;
   unsigned int dims;
   unsigned int maxIndex;
   unsigned long decayRes;
   double* mins;
   double* maxes;
   double eps;
   double removalThreshold;
   double decayFactor;
   double minPts;
   std::priority_queue<QIndex, std::vector<QIndex>, QIndexCompare>** childIndexes;
   Point** childCenters;
   pthread_t thread;
   ReturnCode (*requestFunc)(unsigned int, Request*);
   ReturnCode (*responseFunc)(Response*);

   //ResponseTable childResponses;
};

#endif
