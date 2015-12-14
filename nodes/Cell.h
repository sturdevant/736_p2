
#include <vector>
#include "Point.h"

class Cell {

public:
   Cell();
   ~Cell();
   std::vector<Point*>* getPointVector();
   bool isAssigned();
   bool isShadow();
   bool isFringe();
   bool isInternal();
   void setAssigned(bool flag);
   void setShadow(bool flag);
   void setFringe(bool flag);
   void setTimestamp(unsigned long newTime);
   unsigned long getTimestamp(void);

private:
   std::vector<Point*> vec;
   bool assigned;
   bool shadow;
   bool fringe;
   unsigned long time;

};
