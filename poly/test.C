#include<float.h>
#include<iostream>
#include<cmath>
#include<stdio.h>
#include<stdlib.h>
#include"Vertex.C"
#include<vector>
#include<time.h>
#include<random>

#define N_POINTS 10

/* Generate a random number between fMin & fMax */
double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/* Returns point with smallest coordinate in dimension dim */
Point::Point* get_smallest(std::vector<Point*> v, int dim) {
   int i;
   double minVal = DBL_MAX;
   Point::Point* smallest = v[0];
   for (i = 0; i < v.size(); i++) {
      if (v[i]->getValue()[dim] < minVal) {
         minVal = v[i]->getValue()[dim];
         smallest = v[i];
      }
   }
   return smallest;
}

int main(int argc, char** argv) {
   std::vector<Point*> v;
   srand(time(NULL));
   int i, j;
   double* coords;
   // Fill up v with random points
   for (i = 0; i < N_POINTS; i++) {
      coords = new double[N_DIMENSION];
      for (j = 0; j < N_DIMENSION; j++) { 
         coords[j] = fRand(-10, 10);
      }
      v.push_back(new Point::Point(coords, 1, (unsigned long) time(NULL)));
   }
   Vertex::Vertex* head = new Vertex::Vertex(get_smallest(v, 0));
   //std::cout << "Smallest x: " << head->getPoint()->getValue()[0] << "\n";
   std::cout << "Points:\n";
   // Print out contents of v
   for (i = 0; i < N_POINTS; i++) {
      coords = v[i]->getValue();
      std::cout << i << ":";
      for (j = 0; j < N_DIMENSION - 1; j++)
         std::cout << coords[j] << ",";
      std::cout << coords[N_DIMENSION - 1] << "\n";
   }
   std::vector<Vertex *> vertices;
   // Set a temporary point for head to point back to
   vertices.push_back(head);
   if (head->isIn(v[0])) {
      Vertex::Vertex* temp = new Vertex::Vertex(v[1]);
      vertices.push_back(temp);
      head->setPrev(temp);
      temp->setNext(head);
      //std::cout << "First is smallest!\n";
   } else {
      Vertex::Vertex* temp = new Vertex::Vertex(v[0]);
      vertices.push_back(temp);
      head->setPrev(temp);
      temp->setNext(head);
      //std::cout << "First isn't smallest!\n";
   }
   //std::cout << "Entering while loop\n";
   Vertex::Vertex* curr = head;
   // keep adding vertices (by largest angle spanned) until we get back to head!
   bool done = false;
   while(!done) {
      //std::cout << "Entered while loop!\n";
      Vertex::Vertex* best = new Vertex::Vertex(v[0]);
      //std::cout << "setting next to be address of best!\n";
      curr->setNext(best);
      //std::cout << "setting maxAngle!\n";
      double maxAngle = curr->getAngle();
      //std::cout << "initializing next!\n";
      Vertex::Vertex* next = new Vertex::Vertex(v[0]);
      //std::cout << "entering for!\n";
      for (i = 0; i < N_POINTS; i++) {
         //std::cout << "setting next!\n";
         next = new Vertex::Vertex(v[i]);
         //std::cout << "setting curr's next!\n";
         curr->setNext(next);
         if (curr->getAngle() >= maxAngle) {
            //std::cout << "angle is bigger than max!\n";
            maxAngle = curr->getAngle();
            best = next;
         }
      }
      if (head->getPoint() == best->getPoint()){
         curr->setNext(head);
         head->setPrev(curr);
         done = true;
      } else {
         curr->setNext(best);
         best->setPrev(curr);
         curr = curr->getNext();
         vertices.push_back(best);
      }
   }

   std::cout << "Vertices:\n";
   /*
   for (i = 0; i < vertices.size(); i++) {
      std::cout << vertices[i]->getPoint()->getValue()[0] << ", ";
      std::cout << vertices[i]->getPoint()->getValue()[1] << "\n";
   }
   */
   curr = head;
   do {
      std::cout << curr->getPoint()->getValue()[0] << ", ";
      std::cout << curr->getPoint()->getValue()[1] << "\n";
      curr = curr->getNext();  
   } while (curr != head);
   
   std::cout << "Exiting program!\n";
   return 0;
}
