#include "Request.h"

Request::Request(RequestType type) {
   t = type;
   pt = NULL;
   id = REQUEST_ID_INVALID;
   long1 = 0;
   long2 = 0;
}

Request::Request(RequestType type, Point* pt, unsigned long l1, unsigned long l2) {
   t = type;
   this->pt = pt;
   id = REQUEST_ID_INVALID;
   long1 = l1;
   long2 = l2;
}

Request::Request(Request* req) {
   t = req->t;
   pt = req->pt;
   id = REQUEST_ID_INVALID;
   long1 = req->long1;
   long2 = req->long2;
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

void Request::setLong1(unsigned long l) {
   long1 = l;
}

void Request::setLong2(unsigned long l) {
   long2 = l;
}

unsigned long Request::getLong1() {
   return long1;
}

unsigned long Request::getLong2() {
   return long2;
}
