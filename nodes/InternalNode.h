#ifndef _INTERNAL_NODE_H_
#define _INTERNAL_NODE_H_

#include <cmath>
#include <ctgmath>
#include <iostream>
#include <algorithm>
#include <vector>
#include "Request.h"
#include "ReturnCodes.h"

class InternalNode {

public:

   InternalNode(unsigned int nDims,
                double scale,
                double* mins,
                double* maxes,
                unsigned int nChildren,
                ReturnCode (*requestFunc)(unsigned int, Request*),
                ReturnCode (*responseFunc)(Response*));

   ~InternalNode();
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

   void setOwner*unsigned int* cell, unsigned int owner);
   void assignSubslice(unsigned int* whichDims,
                       unsigned int* cell,
                       unsigned int owner,
                       unsigned int nDims);
   void assignOwners();

   void setLengths();
   void setDimFactors();
   void setMaxIndex();

   unsigned int nOwners;
   unsigned int* owners;
   unsigned int* lengths;
   unsigned int* dimFactors;
   unsigned int dims;
   unsigned int maxIndex;
   double* mins;
   double scale;

   pthread_t thread;
   ReturnCode (*requestFunc)(unsigned int, Request*);
   ReturnCode (*responseFunc)(Response*);

   ResponseTable childResponses;
};
