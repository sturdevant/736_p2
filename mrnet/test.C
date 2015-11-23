#include <iostream>
#include <vector>
#include <stdlib.h>

class Tester {
   public:
      ~Tester() {
         std::cout << "DESTRUCTOR CALLED!\n";
      }
      void print() {
         std::cout << "Test!\n";
      }
};

Tester** testers;

std::vector<Tester*> getTesters() {
   std::vector<Tester*> result;
   for (int i = 0; i < 10; i++) {
      result.push_back(testers[i]);
   }
   return result;
}

int main(int argc, char** argv) {
   testers = (Tester**)malloc(10 * sizeof(Tester*));
   for (int i = 0; i < 10; i++) {
      testers[i] = new Tester();
   }
   for (int j = 0; j < 10; j++) {
      for (int i = 0; i < 10; i++) {
         std::vector<Tester*> vec = getTesters();
         vec[i]->print();
      }
   }
   std::cout << "I made it!\n";
}
