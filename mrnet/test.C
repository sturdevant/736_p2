#include <iostream>
#include <vector>
#include <stdlib.h>
#include "../timedqueue.h"

int main(int argc, char** argv) {
   TimedQueue myQ(sizeof(unsigned int), 2);
   volatile int k = 0;
   int h = 0;
   myQ.add(&h);
   std::cout << "Initial run was " << myQ.remove(&h) << "ns\n";
  
   for (unsigned int i = 0; i < 20; i++) {
      myQ.add(&i);
      for (unsigned int j = 0; j < i * i * 1000; j++) {
         k++;
      }
      std::cout << "Time elapsed on run " << i << " was " << myQ.remove(&i) << "ns\n";
   }
}
