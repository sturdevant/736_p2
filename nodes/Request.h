#ifndef _REQUEST_H_
#define _REQUEST_H_

class Request {
public:
   Request(RequestType type);
   void setID(unsigned long newID);
   void setPoint(double* newPt);
   RequestType getRequestType();
   double* getPoint();
   unsigned long getID();
private:
   RequestType t;
   double* pt;
   unsigned long id;
};

#endif
