

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
   //std::cout << "Response callback called!\n";
   return RETURN_CODE_NO_ERROR;
}

double frand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

main(int argc, char** argv) {

   //srand(time(NULL));
   srand(1);

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
   double eps = 1.25;
   double minPts = 3.0;
   double decay = 0.995;
   unsigned long decayRes = 1;
   double decayThreshold = 0.1;
   double caMaxes[2 * nDims];
   double caMins[2 * nDims];
   double tempMaxes[2 * nDims];
   double tempMins[2 * nDims];

   InternalNode root(nDims, 2, eps, minPts, decayThreshold, decay, decayRes, 
      &mins[0], &maxes[0], 
      &aMins[0], &aMaxes[0], 
      &requestFunc, &responseFunc, 
      &caMins[0], &caMaxes[0]);

   bcopy(caMaxes, tempMaxes, 2 * nDims * sizeof(double));
   bcopy(caMins, tempMins, 2 * nDims * sizeof(double));

   InternalNode in1(nDims, 2, eps, minPts, decayThreshold, decay, decayRes, 
      &mins[0], &maxes[0], 
      &tempMins[0], &tempMaxes[0], 
      &requestFunc, &responseFunc, 
      &caMins[0], &caMaxes[0]);

   /*for (int i = 0; i < 2; i++) {
      std::cout << "Child " << i 
         << "\n\tX: " << caMins[i * nDims + 0] << " to " 
                      << caMaxes[i * nDims + 0] 
         << "\n\tY: " << caMins[i * nDims + 1] << " to " 
                      << caMaxes[i * nDims + 1] << std::endl;
   }*/

   LeafNode n(2, eps, minPts, decay, decayRes, 1, decayThreshold, 
      &mins[0], &maxes[0], 
      &caMins[0], &caMaxes[0], 
      &requestFunc, &responseFunc);

   LeafNode n2(2, eps, minPts, decay, decayRes, 2, decayThreshold, 
      &mins[0], &maxes[0], 
      &caMins[1 * nDims], &caMaxes[1 * nDims], 
      &requestFunc, &responseFunc);
  
   InternalNode in2(nDims, 2, eps, minPts, decayThreshold, decay, decayRes, 
      &mins[0], &maxes[0], 
      &tempMins[1 * nDims], &tempMaxes[1 * nDims], 
      &requestFunc, &responseFunc, 
      &caMins[0], &caMaxes[0]);

   LeafNode n3(2, eps, minPts, decay, decayRes, 3, decayThreshold, 
      &mins[0], &maxes[0], 
      &caMins[0], &caMaxes[0], 
      &requestFunc, &responseFunc);

   LeafNode n4(2, eps, minPts, decay, decayRes, 4, decayThreshold, 
      &mins[0], &maxes[0], 
      &caMins[1 * nDims], &caMaxes[1 * nDims], 
      &requestFunc, &responseFunc);
   
   double setCoords[2][5];

   setCoords[0][0] = -1.60;
   setCoords[1][0] = 0;

   setCoords[0][1] = -1.61;
   setCoords[1][1] = 0;
   
   setCoords[0][2] = -1.62;
   setCoords[1][2] = 0;
   
   setCoords[0][3] = 1.59;
   setCoords[1][3] = 0;
   
   setCoords[0][4] = 1.58;
   setCoords[1][4] = 0;
   
   setCoords[0][5] = 1.57;
   setCoords[1][5] = 0;
   
   setCoords[0][6] = 0.6;
   setCoords[1][6] = 0;
   
   setCoords[0][7] = -0.63;
   setCoords[1][7] = 0;
   
   int runs = 12490;

   double coords[2];
   for (int i = 0; i < runs; i++) {
      coords[0] = frand(-10.0, 10.0);//setCoords[0][i];
      coords[1] = frand(-10.0, 10.0);//setCoords[1][i];
      Point pt(coords, 1.0, i);
      Request req(REQUEST_TYPE_ADD_POINT);
      Response res((ResponseType)0, 1);
      req.setPoint(&pt);
      n.query(&req, &res);

      std::cout << "Sending second query!\n";
      //Point pt2(coords, 1.0, i);
      //Request req2(REQUEST_TYPE_ADD_POINT);
      //Response res2;
      //req2.setPoint(&pt2);
      //std::cout << "Calling query!\n";
      n2.query(&req, &res);

      std::cout << "Sending 3rd query!\n";
      n3.query(&req, &res);

      std::cout << "Sending 4th query!\n";
      n4.query(&req, &res);

      while (requestList.size() > 0) {
         std::cout << "\nBroadcasting request (t = " << i << ")!\n\n";
         n.query(requestList[0], &res);
         std::cout << "\n\tFirst response!\n\n";
         n2.query(requestList[0], &res);
         std::cout << "\n\tSecond response!\n\n";
         n3.query(requestList[0], &res);
         std::cout << "\n\tThird response!\n\n";
         n4.query(requestList[0], &res);
         std::cout << "\n\tFourth response!\n\n";
         requestList.erase(requestList.begin());
      }


   }

   std::cout << "Taking grid!\n";
   
   FILE* gridFile = fopen("grid", "w");
   for (coords[0] = -10; coords[0] < 10; coords[0] += 0.2) {
      for (coords[1] = -10; coords[1] < 10; coords[1] += 0.2) {
         Request getReq(REQUEST_TYPE_POINT_DATA);
         Point pt(coords, 1.0, runs);
         getReq.setPoint(&pt);
         Response res((ResponseType)0, 1);
         res.setClusterId(0);
         n.query(&getReq, &res);
         unsigned long clustId = res.getClusterId();
         fprintf(gridFile, "%ld:%lf,%lf,%ld\n", runs, coords[0], coords[1], clustId);
         res.setClusterId(0);
         n2.query(&getReq, &res);
         clustId = res.getClusterId();
         fprintf(gridFile, "%ld:%lf,%lf,%ld\n", runs, coords[0], coords[1], clustId);
         res.setClusterId(0);
         n3.query(&getReq, &res);
         clustId = res.getClusterId();
         fprintf(gridFile, "%ld:%lf,%lf,%ld\n", runs, coords[0], coords[1], clustId);
         res.setClusterId(0);
         n4.query(&getReq, &res);
         clustId = res.getClusterId();
         fprintf(gridFile, "%ld:%lf,%lf,%ld\n", runs, coords[0], coords[1], clustId);
      }
   }
   fclose(gridFile);
         
   std::cout << "Taking snapshot!\n";

   FILE* file = fopen("snapshot1", "w");
   FILE* file2 = fopen("assigns", "w");
   n.snapshot(file, file2, runs - 1);
   fclose(file);
   fclose(file2);

   file = fopen("snapshot2", "w");
   file2 = fopen("assigns2", "w");
   n2.snapshot(file, file2, runs - 1);
   fclose(file);
   fclose(file2);

   file = fopen("snapshot3", "w");
   file2 = fopen("assigns3", "w");
   n3.snapshot(file, file2, runs - 1);
   fclose(file);
   fclose(file2);

   file = fopen("snapshot4", "w");
   file2 = fopen("assigns4", "w");
   n4.snapshot(file, file2, runs - 1);
   fclose(file);
   fclose(file2);
   //*/ 
   //std::cout << "Resulting point weight = " << res.getValue() << std::endl;

   return 0;
}
