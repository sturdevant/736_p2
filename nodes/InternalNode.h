#ifndef _INTERNAL_NODE_H_
#define _INTERNAL_NODE_H_

#include <cmath>
#include <ctgmath>
#include <iostream>
#include <algorithm>
#include "ErrorCodes.h"

class InternalNode {

public:

InternalNode(unsigned int nDims, double scale, double* mins, double* maxes, unsigned int nChildren, ErrorCode (*requestFunc)(unsigned, Request*));
~InternalNode();
ErrorCode addPoint(Point*);
Response* query(Request* req);
Request* update(Response* res);

private:

   unsigned int getPointOwner(double* pt);
   void assignOwners();
   unsigned int nOwners;
   unsigned int* owners;
   double* mins;
   unsigned int* lengths;
   unsigned int* dimFactor;
   double scale;
   unsigned int dims;
   unsigned int maxIndex;
};
