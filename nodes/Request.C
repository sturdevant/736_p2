#include "Request.h"

Request::Request(RequestType type) {
   t = type;
   pt = NULL;
   id = INVALID_ID;
}

void Request::setID(unsigned long newID) {
   id = newID;
}

void Request::setPoint(double* newPt) {
   pt = newPt;
}

RequestType Request::getRequestType() {
   return t;
}

double* Request::getPoint() {
   return pt;
}

unsigned long Request::getID() {
   return id;
}
