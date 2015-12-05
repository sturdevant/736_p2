
#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include "ResponseType.h"

#define RESPONSE_FORMAT_STRING "%ld %ld %ld %lf %ld %ld"

class Response {
public:
   Response(ResponseType t, unsigned long id);
   ~Response();
   void setType(ResponseType t);
   void setValue(double v);
   double getValue();
   unsigned long getClusterId();
   void setClusterId(unsigned long clustId);
   unsigned long getId() { return id; }
   unsigned long getTime() { return time; }
   void setTime(unsigned long newTime);
   void unpack(
      unsigned long* uType,
      unsigned long* uId,
      unsigned long* uTime,
      double* uValue,
      unsigned long* uClusterId
   );

private:
   ResponseType type;
   unsigned long id;
   unsigned long time;
   double value;
   unsigned long clusterId;
};

#endif // _RESPONSE_H_

