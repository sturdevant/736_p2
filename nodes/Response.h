
#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include "ResponseType.h"

class Response {
public:
   Response();
   ~Response();
   void setType(ResponseType t);
   void setValue(double v);
   double getValue();

private:
   ResponseType type;
   double value;
};

#endif // _RESPONSE_H_

