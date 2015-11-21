
#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <strings.h>
#include "../nodes/Point.C"

class Vertex {

public:
   Vertex(Point::Point* p);
   ~Vertex(void);
   void setNext(Vertex::Vertex* p);
   void setPrev(Vertex::Vertex* p);
   Vertex::Vertex* getPrev();
   Vertex::Vertex* getNext();
   Point::Point* getPoint();
   double getAngle();
   bool isIn(Point::Point* p);
   bool isBehind(Point::Point* p);
   bool isAhead(Point::Point* p);
   bool hasPrev();
   bool hasNext();
private:
   Point::Point* myPoint;
   Vertex::Vertex* next;
   Vertex::Vertex* prev;
   double dot_p(double* p, double* q);
   double* nOff;
   double* pOff;
   bool hasP;
   bool hasN;

};

#endif // _VERTEX_H_
