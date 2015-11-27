
#include "Cell.h"

Cell::Cell() {
   assigned = false;
}

Cell::~Cell() {
}

bool Cell::isAssigned() {
   return assigned;
}

bool Cell::isShadow() {
   return shadow;
}

bool Cell::isFringe() {
   return fringe;
}

bool Cell::isInternal() {
   return isAssigned() && !isShadow() && !isFringe();
}

void Cell::setAssigned(bool flag) {
   if (!flag) {
      shadow = false;
      fringe = false;
   }
   assigned = flag;
}

void Cell::setShadow(bool flag) {
   if (flag) {
      assigned = true;
      fringe = false;
   }
   shadow = flag;
}

void Cell::setFringe(bool flag) {
   if (flag) {
      assigned = true;
      shadow = false;
   }
   fringe = flag;
}

std::vector<Point*>* Cell::getPointVector() {
   return &vec;
}
