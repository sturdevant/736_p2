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
   void setID(unsigned long newID);
   void setPoint(Point* newPt);
   RequestType getRequestType();
   Point* getPoint();
   unsigned long getID();
private:
   RequestType t;
   Point* pt;
   unsigned long id;
};

#endif
