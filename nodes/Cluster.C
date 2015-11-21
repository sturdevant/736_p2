
#include "Cluster.h"

Cluster::Cluster(Point* pt, unsigned long id) {
   this->id = id;
   this->ptCount = 1;
}

Cluster::~Cluster() {
   for (int i = 0; i < checkPoints.size(); i++) {
      delete checkPoints[i];
   }
}

double Cluster::getSqrDistToCheckPoint(Point* pt, int* retIndex) {
   double minDistSqr = DBL_MAX;
   int closestIndex = -1;
   for (int i = 0; i < checkPoints.size(); i++) {
      double dSqr = pt->getDistSqr(checkPoints[i]);
      if (dSqr < minDistSqr) {
         minDistSqr = dSqr;
         closestIndex = i;
      }
   }
   if (retIndex != NULL) {
      return *retIndex = closestIndex;
   }

   return minDistSqr;
}

void Cluster::addCheckPoint(Point* pt) {
   checkPoints.push_back(new Point(*pt));
}

void Cluster::moveCheckPoint(int index, Point* newPt) {
   delete checkPoints[index];
   checkPoints.erase(checkPoints.begin() + index);
   checkPoints.push_back(new Point(*newPt));
}

void Cluster::moveCheckPoint(Point* oldPt, Point* newPt) {
   for (int i = 0; i < checkPoints.size(); i++) {
      if (!memcmp(oldPt->getValue(), checkPoints[i]->getValue(), N_DIMENSION * sizeof(double))) {
         delete checkPoints[i];
         checkPoints.erase(checkPoints.begin() + i);
         checkPoints.push_back(new Point(*newPt));
         i--;
      }
   }
}

void Cluster::removeCheckPoint(int index) {
   delete checkPoints[index];
   checkPoints.erase(checkPoints.begin() + index);
}

void Cluster::removeCheckPoint(Point* pt) {
   for (int i = 0; i < checkPoints.size(); i++) {
      if (!memcmp(pt->getValue(), checkPoints[i]->getValue(), N_DIMENSION * sizeof(double))) {
         delete checkPoints[i];
         checkPoints.erase(checkPoints.begin() + i);
         i--;
      }
   }
}

void Cluster::addPt(Point* pt) {
   ptCount++;
}

int Cluster::removePt(Point* pt) {
   ptCount--;
   //std::cout << "Cluster now has " << ptCount << " points\n";
   if (ptCount == 0) {
      //std::cout << "CLUSTER DELETING SELF...\n";
      delete this;
      return -1;
   }
   return 0;
}

