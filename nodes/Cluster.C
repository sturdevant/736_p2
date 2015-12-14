
#include "Cluster.h"

Cluster::Cluster(Point* pt, unsigned long id) {
   this->id = id;
   this->ptCount = 0;
}

Cluster::Cluster(unsigned long id) {
   this->id = id;
   this->ptCount = 0;
}

Cluster::~Cluster() {
   for (int i = 0; i < checkPoints.size(); i++) {
      delete checkPoints[i];
   }
}

void Cluster::printCheckPoints() {
   //std::cout << "Cluster " << id << " CHECKPOINTS:\n";
   for (unsigned int i = 0; i < checkPoints.size(); i++) {
      Point* pt = checkPoints[i];
      //std::cout << "\t" << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
   }
}

double Cluster::getSqrDistToCheckPoint(Point* pt, int* retIndex) {
   double minDistSqr = DBL_MAX;
   int closestIndex = -1;
   //std::cout << "Computing min distance to " << checkPoints.size() << " points!\n";
   for (int i = 0; i < checkPoints.size(); i++) {
      double dSqr = pt->getDistSqr(checkPoints[i]);
      //std::cout << "Got distance from pt " << i << "\n";
      if (dSqr < minDistSqr) {
         minDistSqr = dSqr;
         closestIndex = i;
      }
   }
   if (retIndex != NULL) {
      *retIndex = closestIndex;
   }

   return minDistSqr;
}

void Cluster::addCheckPoint(Point* pt) {
   //std::cout << "Checkpoint added!\n";
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
   return ptCount;
   //std::cout << "Cluster now has " << ptCount << " points\n";
   //if (ptCount == 0) {
   //   std::cout << "CLUSTER DELETING SELF...\n";
   //   delete this;
   //   return -1;
   //}
   //return 0;
}

