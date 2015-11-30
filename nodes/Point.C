
#include "Point.h"

double Point::eps;
double Point::epsSqr;
double Point::decayFactor;
double Point::decayRes;


Point::Point(double* vals, double weight, unsigned long timestamp) {
   this->clust = NULL;
   this->val = (double*)malloc(N_DIMENSION * sizeof(double));
   bcopy(vals, this->val, N_DIMENSION * sizeof(double));
   this->weight = weight;
   this->timestamp = timestamp;
   this->nWeight = weight;
   this->visited = 0;
}

Point::Point(Point& p) {
   this->weight = p.weight;
   this->nWeight = p.nWeight;
   this->timestamp = p.timestamp;
   this->val = (double*)malloc(N_DIMENSION * sizeof(double));
   bcopy(p.val, this->val, N_DIMENSION * sizeof(double));
   this->clust = p.clust;
   this->visited = p.visited;
}

Point::~Point() {
   free(val);
}

bool Point::isNeighbor(Point* pt) {
   double distSqr = 0;
   for (unsigned int i = 0; distSqr < epsSqr && i < N_DIMENSION; i++) {
      distSqr += (val[i] - pt->val[i]) * (val[i] - pt->val[i]);
   }
   return distSqr < epsSqr;
}
   
double Point::addNWeight(double amtToAdd, unsigned long time) {
   double timeFactor = getTimeFactor(timestamp, time);
   nWeight = timeFactor * nWeight + amtToAdd;
   weight = timeFactor * weight;
   timestamp = time;
   return nWeight;
}

double Point::getWeight(unsigned long time) {
   return weight * getTimeFactor(timestamp, time);
}

Cluster* Point::getCluster() {
   return clust;
}

void Point::setCluster(Cluster* newClust) {
   clust = newClust;
}

double Point::getNWeight(unsigned long time) {
   return nWeight * getTimeFactor(timestamp, time);
}

void Point::setNWeight(double newNWeight) {
   nWeight = newNWeight;
}

unsigned long Point::getTimestamp() {
   return timestamp;
}

void Point::setTimestamp(unsigned long time) {
   this->timestamp = time;
}

void Point::setWeight(double newWeight) {
   weight = newWeight;
}

double* Point::getValue() {
   return val;
}

void Point::setValue(double* newValue) {
   bcopy(newValue, this->val, N_DIMENSION * sizeof(double));
}

unsigned long Point::getVisited() {
   return visited;
}

void Point::setVisited(unsigned long newVisited) {
   visited = newVisited;
}

double Point::getTimeFactor(unsigned long t1, unsigned long t2) {
   double result = 1.0;
   if (t2 <= t1) {
      return 1.0;
   }
   unsigned long nMultiples = (t2 - t1) / decayRes;
   for (; nMultiples > 0; nMultiples--) {
      result *= decayFactor;
   }
   return result;
}

double Point::getDistSqr(Point* pt) {
   double distSqr = 0;
   for (unsigned int i = 0; i < N_DIMENSION; i++) {
      distSqr += (val[i] - pt->val[i]) * (val[i] - pt->val[i]);
   }
   return distSqr;
}

void Point::setDecayFactor(double newFactor, unsigned long newRes) {
   decayFactor = newFactor;
   decayRes = newRes;
}

void Point::setEpsilon(double newEps) {
   eps = newEps;
   epsSqr = newEps * newEps;
}

bool operator> (const QPoint& qp1, const QPoint& qp2) {
   return qp1.dist > qp2.dist;
}

bool operator< (const QPoint& qp1, const QPoint& qp2) {
   return qp1.dist < qp2.dist;
}

struct QPointCompare : public std::binary_function<QPoint&, QPoint&, bool> {
   bool operator()(const QPoint& qp1, const QPoint& qp2) const {
      return qp1.dist < qp2.dist;
   }
};
