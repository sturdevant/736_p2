
#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include <string.h>
#include <float.h>
#include <vector>
#include "Point.h"
#include <iostream>

#define N_DIMENSION 2

class Point;

class Cluster {
public:
   Cluster(Point* pt, unsigned long id);
   Cluster(unsigned long id);
   ~Cluster();
   void addPt(Point* pt);
   void printCheckPoints();
   int removePt(Point* pt);
   unsigned long getId() { return id; }
   unsigned long getPtCount() { return ptCount; }
   double getSqrDistToCheckPoint(Point* pt, int* retIndex);
   void addCheckPoint(Point* pt);
   void moveCheckPoint(int index, Point* newPt);
   void moveCheckPoint(Point* oldPt, Point* newPt);
   void removeCheckPoint(int index);
   void removeCheckPoint(Point* pt);

private:
   unsigned long ptCount;
   unsigned long id;
   std::vector<Point*> checkPoints;

};

#endif // _CLUSTER_H_
