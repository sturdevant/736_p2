

#include <iostream>
#include "LeafNode.h"
#include "ReturnCode.h"
#include "InternalNode.h"

ReturnCode requestFunc(unsigned int, Request* req) {
   std::cout << "Request callback called!\n";
   return RETURN_CODE_NO_ERROR;
}

ReturnCode responseFunc(Response* res) {
   std::cout << "Response callback called!\n";
   return RETURN_CODE_NO_ERROR;
}

int main(int argc, char** argv) {

   double mins[2];
   mins[0] = -10;
   mins[1] = -10;

   double maxes[2];
   maxes[0] = 10;
   maxes[1] = 10;

   double aMins[2];
   aMins[0] = -7;
   aMins[1] = -1;

   double aMaxes[2];
   aMaxes[0] = 8;
   aMaxes[1] = 0;

   std::cout << "Initializing leafnode...\n";

   double caMins[4][2];
   double caMaxes[4][2];

   InternalNode in(2, 4, 1.0, 3.0, 0.1, 0.90, &mins[0], &maxes[0], &aMins[0], &aMaxes[0], &requestFunc, &responseFunc, (double**)&caMins[0][0], (double**)&caMaxes[0][0]);

   LeafNode n(2, 1, 1, 0.70, &mins[0], &maxes[0], &aMins[0], &aMaxes[0], &requestFunc, &responseFunc);

   double* vals = (double*)malloc(2 * sizeof(double));
   vals[0] = 0;
   vals[1] = 0;
   double* vals2 = (double*)malloc(2 * sizeof(double));
   vals2[0] = 1.5;
   vals2[1] = 0.0;
   double* vals3 = (double*)malloc(2 * sizeof(double));
   vals3[0] = .75;
   vals3[1] = 0;

   std::cout << "Setting up point...\n";

   Point::setDecayFactor(.9, 1);
   Point::setEpsilon(1.0);

   Point pt3(&vals3[0], 1.0, 14);
   Point pt2(&vals2[0], 1.0, 12);
   Point pt(&vals[0], 1.0, 10);

   std::cout << "Setting up requests...\n";

   Request addReq(REQUEST_TYPE_ADD_POINT), getReq(REQUEST_TYPE_POINT_DATA);
   Request addReq2(REQUEST_TYPE_ADD_POINT);
   Request addReq3(REQUEST_TYPE_ADD_POINT);

   std::cout << "Setting up add request...\n";

   addReq.setPoint(&pt);
   addReq2.setPoint(&pt2);
   addReq3.setPoint(&pt3);

   std::cout << "Setting up get request...\n";

   getReq.setPoint(&pt);

   std::cout << "Sending add query...\n";

   Response res;
   n.query(&addReq, &res);
   n.query(&addReq, &res);
   n.query(&addReq, &res);
   n.query(&addReq, &res);
   n.query(&addReq, &res);
   n.query(&addReq2, &res);
   n.query(&addReq2, &res);
   n.query(&addReq2, &res);
   n.query(&addReq2, &res);
   n.query(&addReq2, &res);
   n.query(&addReq3, &res);

   std::cout << "Sending get query...\n";
   n.query(&getReq, &res);

   FILE* file = fopen("snapshot", "w");
   FILE* file2 = fopen("assigns", "w");
   n.snapshot(file, file2);
   fclose(file);
   fclose(file2);

   std::cout << "Resulting point weight = " << res.getValue() << std::endl;

   return 0;
}
