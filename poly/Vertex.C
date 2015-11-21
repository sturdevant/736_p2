#include "Vertex.h"
#include <cmath>

Vertex::Vertex(Point* p) {
   this->myPoint = p;
   this->pOff = new double[N_DIMENSION];
   this->nOff = new double[N_DIMENSION];
   this->hasP = false;
   this->hasN = false;
}

Vertex::~Vertex() {
   std::cout << "This vertex is being freed!\n";
   std::cout << myPoint->getValue()[0] << "," << myPoint->getValue()[1] << "\n";
   delete this->pOff;
   delete this->nOff;
}

void Vertex::setNext(Vertex::Vertex* p) {
   this->next = p;
   int i;
   for (i = 0; i < N_DIMENSION; i++) {
      this->nOff[i] = this->next->getPoint()->getValue()[i] - this->myPoint->getValue()[i];
   }
   this->hasN = true;
}

void Vertex::setPrev(Vertex::Vertex* p) {
   this->prev = p;
   int i;
   for (i = 0; i < N_DIMENSION; i++) {
      this->pOff[i] = this->prev->getPoint()->getValue()[i] - this->myPoint->getValue()[i];
   }
   this->hasP = true;
}

Point::Point* Vertex::getPoint() {
   return this->myPoint;
}

Vertex::Vertex* Vertex::getNext() {
   return this->next;
}

Vertex::Vertex* Vertex::getPrev() {
   return this->prev;
}

double Vertex::dot_p(double* p, double* q) {
   int i;
   double sum = 0;
   for (i = 0; i < N_DIMENSION; i++) {
      sum += p[i] * q[i];
   }
   return sum;
}

/* returns angle spanned by this vertex */
double Vertex::getAngle() {
   if (!this->hasPrev()  || !this->hasNext()) {
      return -1;
   }
   double mag_p, mag_n;
   mag_p = sqrt(dot_p(pOff, pOff));
   mag_n = sqrt(dot_p(nOff, nOff));
   if (mag_p == 0 || mag_n == 0)
      return 0;
   return acos(dot_p(nOff, pOff) / (mag_p * mag_n));
}


bool Vertex::isIn(Point::Point* p) {
   // Return true if the point is this one, ahead or behind this one
   return p == myPoint || isBehind(p) || isAhead(p);
}

bool Vertex::hasNext() {
   if (this->hasN){
      return true;
   }
   return false;
}

bool Vertex::isAhead(Point::Point*p) {
   Vertex::Vertex* curr = this;
   while (curr->hasNext()) {
      curr = curr->getNext();
      if (curr->getPoint() == p) {
         return true;
      }
   }
   return false;
}

bool Vertex::hasPrev() {
   if (this->hasP){
      return true;
   }
   return false;
}

bool Vertex::isBehind(Point::Point* p) {
   Vertex::Vertex* curr = this;
   while (curr->hasPrev()) {
      curr = curr->getPrev();
      if (curr->getPoint() == p) {
         return true;
      }
   }
   return false;
}
