#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <limits.h>
#include "RequestType.h"
#include "Point.h"
#include <stdlib.h>

#define REQUEST_ID_INVALID ULLONG_MAX

class Request {
public:
   Request(RequestType type);
   Request(RequestType type, Point* pt, unsigned long l1, unsigned long l2);
   Request(Request* req);
   void setID(unsigned long newID);
   void setPoint(Point* newPt);
   RequestType getRequestType();
   Point* getPoint();
   unsigned long getID();
   void setLong1(unsigned long l);
   void setLong2(unsigned long l);
   unsigned long getLong1();
   unsigned long getLong2();

private:
   RequestType t;
   Point* pt;
   unsigned long id;
   unsigned long long1;
   unsigned long long2;
};

#endif
