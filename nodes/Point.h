
#ifndef _POINT_H_
#define _POINT_H_

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <iostream>
#include <functional>
#include "ClusterId.h"
#include "Cluster.h"

#define N_DIMENSION 2
#define MIN_PTS 3

class Cluster;

class Point {

public:
   Point(double* vals, double weight, unsigned long timestamp);
   Point(Point& p);
   ~Point(void);
   double getDistSqr(Point* pt);
   bool isNeighbor(Point* pt);
   double* getValue();
   void setValue(double*);
   double getWeight(unsigned long time);
   double addNWeight(double amtToAdd, unsigned long time);
   Cluster* getCluster();
   void setCluster(Cluster* newClust);
   double getNWeight(unsigned long time);
   void setNWeight(double newNWeight);
   unsigned long getTimestamp();
   void setTimestamp(unsigned long time);
   void setWeight(double newWeight);
   unsigned long getVisited();
   void setVisited(unsigned long newVisited);
   static void setEpsilon(double newEps);
   static void setDecayFactor(double newFactor, unsigned long newRes);
   static double getTimeFactor(unsigned long t1, unsigned long t2);

private:
   double* val;             // In packet double* of size N_DIMENSION (read from 
                            // socket/client).
   double weight;           // In packet double (incoming, 1)
   unsigned long timestamp; // In packet unsigned long (incoming, set to time)
   unsigned long visited;   // NOT IN PACKET, internal use only
   Cluster* clust;          // In packet, unsigned long cluster id 
                            // (clust->getId() (incoming, 0)
   double nWeight;          // In packet, double (incoming, 0)
   static double eps;
   static double epsSqr;
   static double decayFactor;
   static double decayRes;

};

typedef struct QPoint {
   Point* pt;
   double dist;
} QPoint;

//struct QPointCompare;// : public std::binary_function<QPoint&, QPoint&, bool> {

#endif // _POINT_H_
