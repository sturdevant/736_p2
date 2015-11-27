

#include <iostream>
#include "LeafNode.h"
#include "ReturnCode.h"
#include "InternalNode.h"

std::vector<Request*> requestList;

ReturnCode requestFunc(unsigned int, Request* req) {
   //std::cout << "Request callback called!\n";
   Request* r = new Request(req);
   requestList.push_back(r);
   //std::cout << "Request added to list!\n";
   return RETURN_CODE_NO_ERROR;
}

ReturnCode responseFunc(Response* res) {
   std::cout << "Response callback called!\n";
   return RETURN_CODE_NO_ERROR;
}

double frand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

main(int argc, char** argv) {

   srand(time(NULL));

   double mins[2];
   mins[0] = -10;
   mins[1] = -10;

   double maxes[2];
   maxes[0] = 10;
   maxes[1] = 10;

   double aMins[2];
   aMins[0] = -10;
   aMins[1] = -10;

   double aMaxes[2];
   aMaxes[0] = 10;
   aMaxes[1] = 10;

   std::cout << "Initializing leafnode...\n";


   int nDims = 2;
   double eps = 2.5;
   double minPts = 3.0;
   double decay = 0.99;
   unsigned long decayRes = 20;
   double decayThreshold = 0.1;
   double caMaxes[2 * nDims];
   double caMins[2 * nDims];

   InternalNode in(nDims, 2, eps, minPts, decayThreshold, decay, decayRes, &mins[0], &maxes[0], &aMins[0], &aMaxes[0], &requestFunc, &responseFunc, &caMins[0], &caMaxes[0]);

   for (int i = 0; i < 2; i++) {
      std::cout << "Child " << i 
         << "\n\tX: " << caMins[i * nDims + 0] << " to " 
                      << caMaxes[i * nDims + 0] 
         << "\n\tY: " << caMins[i * nDims + 1] << " to " 
                      << caMaxes[i * nDims + 1] << std::endl;
   }

   LeafNode n(2, eps, minPts, decay, decayRes, 1, decayThreshold, &mins[0], &maxes[0], &caMins[0], &caMaxes[0], &requestFunc, &responseFunc);

   LeafNode n2(2, eps, minPts, decay, decayRes, 2, decayThreshold, &mins[0], &maxes[0], &caMins[1 * nDims], &caMaxes[1 * nDims], &requestFunc, &responseFunc);
   
   double setCoords[2][5];

   setCoords[0][0] = -1.0;
   setCoords[1][0] = 0;

   setCoords[0][1] = -1.04;
   setCoords[1][1] = 0;
   
   setCoords[0][2] = -1.05;
   setCoords[1][2] = 0;
   
   setCoords[0][3] = -1.06;
   setCoords[1][3] = 0;
   
   setCoords[0][4] = -1.07;
   setCoords[1][4] = 0;
   
   int runs = 4;
   double coords[2];
   for (int i = 0; i < runs; i++) {
      coords[0] = setCoords[0][i];//frand(-10.0, 10.0);
      coords[1] = setCoords[1][i];//frand(-10.0, 10.0);
      Point pt(coords, 1.0, i);
      Request req(REQUEST_TYPE_ADD_POINT);
      Response res;
      req.setPoint(&pt);
      n.query(&req, &res);

      std::cout << "Sending second query!\n";
      Point pt2(coords, 1.0, i);
      Request req2(REQUEST_TYPE_ADD_POINT);
      Response res2;
      req2.setPoint(&pt2);
      //std::cout << "Calling query!\n";
      n2.query(&req2, &res2);

      while (requestList.size() > 0) {
         std::cout << "Broadcasting request (t = " << i << ")!\n";
         n.query(requestList[0], &res);
         std::cout << "\tFirst response!\n";
         n2.query(requestList[0], &res);
         std::cout << "\tSecond response!\n";
         requestList.erase(requestList.begin());
      }


   }

/*
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
*/
   FILE* file = fopen("snapshot", "w");
   FILE* file2 = fopen("assigns", "w");
   n.snapshot(file, file2);
   fclose(file);
   fclose(file2);

   file = fopen("snapshot2", "w");
   file2 = fopen("assigns2", "w");
   n2.snapshot(file, file2);
   fclose(file);
   fclose(file2);

   /*
   FILE* gridFile = fopen("grid", "w");
   for (coords[0] = -10; coords[0] < 10; coords[0] += 0.4) {
      for (coords[1] = -10; coords[1] < 10; coords[1] += 0.4) {
         Request getReq(REQUEST_TYPE_POINT_DATA);
         Point pt(coords, 1.0, runs);
         getReq.setPoint(&pt);
         Response res;
         n.query(&getReq, &res);
         unsigned long clustId = res.getClusterId();
         fprintf(gridFile, "%ld:%lf,%lf,%ld\n", runs, coords[0], coords[1], clustId);
      }
   }
   fclose(gridFile);

   gridFile = fopen("grid2", "w");
   for (coords[0] = -10; coords[0] < 10; coords[0] += 0.4) {
      for (coords[1] = -10; coords[1] < 10; coords[1] += 0.4) {
         Request getReq(REQUEST_TYPE_POINT_DATA);
         Point pt(coords, 1.0, runs);
         getReq.setPoint(&pt);
         Response res;
         n2.query(&getReq, &res);
         unsigned long clustId = res.getClusterId();
         fprintf(gridFile, "%ld:%lf,%lf,%ld\n", runs, coords[0], coords[1], clustId);
      }
   }
   fclose(gridFile);
   */
         
   //std::cout << "Resulting point weight = " << res.getValue() << std::endl;

   return 0;
}
