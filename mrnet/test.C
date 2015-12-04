#include <iostream>
#include <vector>
#include <stdlib.h>
#include "../timedqueue.h"

int main(int argc, char** argv) {
   TimedQueue myQ(sizeof(unsigned int), 20000);
   unsigned int i;
  
   for (i = 0; i < 3; i++) {
      myQ.add(&i);
   }

   for (i = 0; i < 3; i++) {
      myQ.remove(&i);
   }
}
