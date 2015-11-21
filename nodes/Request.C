#include "Request.h"

Request::Request(RequestType type) {
   t = type;
   pt = NULL;
   id = REQUEST_ID_INVALID;
}

void Request::setID(unsigned long newID) {
   id = newID;
}

void Request::setPoint(Point* newPt) {
   pt = newPt;
}

RequestType Request::getRequestType() {
   return t;
}

Point* Request::getPoint() {
   return pt;
}

unsigned long Request::getID() {
   return id;
}
