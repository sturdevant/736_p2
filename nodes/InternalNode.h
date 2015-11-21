#ifndef _INTERNAL_NODE_H_
#define _INTERNAL_NODE_H_

#include <cmath>
//#include <ctgmath>
#include <iostream>
#include <algorithm>
#include <vector>
#include "Request.h"
#include "Response.h"
#include "ReturnCode.h"
#include "Point.h"

class InternalNode {

public:

   InternalNode(unsigned int nDims,
                unsigned int nChildren,
                double epsilon,
                double minPts,
                double removalThreshold,
                double decayFactor,
                double* mins,
                double* maxes,
                double* aMins, 
                double* aMaxes,
                ReturnCode (*requestFunc)(unsigned int, Request*),
                ReturnCode (*responseFunc)(Response*),
                double** caMins,
                double** caMaxes);

   ~InternalNode();
   bool admitpoint(Point* pt);
   ReturnCode addPoint(Request* req);
   ReturnCode query(Request* req);
   ReturnCode update(Response* res);

private:
   
   unsigned int getPointOwner(double* pt);
   void getPointOwnerGroup(double* pt,
                           std::vector<unsigned int>& uniqueOwners);
   unsigned int getCellOwner(unsigned int* cell);
   void getCellOwnerGroup(unsigned int* whichDims,
                          unsigned int* cell,
                          std::vector<unsigned int>& uniqueOwners,
                          unsigned int nDims);

   void setOwner(unsigned int* cell, unsigned int owner);
   void assignSubslice(unsigned int* whichDims,
                       unsigned int* cell,
                       unsigned int owner,
                       unsigned int nDims);
   void assignOwners(double** caMins, double** caMaxes);

   void setLengths();
   void setDimFactors();
   void setMaxIndex();

   unsigned int nChildren;
   unsigned int* owners;
   unsigned int* lengths;
   unsigned int* dimFactors;
   unsigned int dims;
   unsigned int maxIndex;
   double* mins;
   double* maxes;
   double eps;
   double removalThreshold;
   double decayFactor;
   double minPts;

   pthread_t thread;
   ReturnCode (*requestFunc)(unsigned int, Request*);
   ReturnCode (*responseFunc)(Response*);

   //ResponseTable childResponses;
};

#endif
