/****************************************************************************
 * Copyright © 2003-2015 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/
#include <stdio.h>
#include <vector>
#include "mrnet/MRNet.h"
#include "mrnet/Packet.h"
#include "mrnet/NetworkTopology.h"
#include "ReturnCode.h"
#include "Request.h"
#include "InternalNode.h"

extern "C" {


ReturnCode fakeReqFunc(unsigned int, Request*) {
   std::cout << "Request func called!\n";
}

ReturnCode fakeResFunc(Response* res) {
   std::cout << "Response func called!\n";
}

bool isLeaf = false;
const MRN::Network* network = NULL;
MRN::Stream** streams = NULL;
MRN::Communicator** comms = NULL;
MRN::Rank* childRanks = NULL;
InternalNode* internalNode;
const char* PointMux_format_string = "%f%f";
void PointMux(std::vector< MRN::PacketPtr >& packets_in,
              std::vector< MRN::PacketPtr >& packets_out,
              std::vector< MRN::PacketPtr >& /* packets_out_reverse */,
              void ** /* client data */,
		        MRN::PacketPtr& /* params */,
              const MRN::TopologyLocalInfo& t)
{

   float x, y;
   int i;
   packets_in[0]->unpack(PointMux_format_string, &x, &y);

   if (!isLeaf) {
      if (x < 0 && y < 0) {
         i = 0;
      } else if (x < 0) {
         i = 1;
      } else if (x > 0 && y < 0) {
         i = 2;
      } else {
         i = 3;
      }
      streams[i]->send(packets_in[0]->get_Tag(), PointMux_format_string, x, y);
   } else {
      printf("Rank: %d got point %f, %f\n", (int)t.get_Rank(), x, y);
   }
}
/*
 * Could consider creating packaging code as shown here.
PacketPtr getRequestPacket(Request* req, unsigned int stream_id) {
   return new Packet(stream_id, 1994, "%ld", 7);
}
*/

ReturnCode requestHandler(unsigned int child, Request* req) {
   //network->send(childRanks[child], 1994, "%ld", 7);
   //PacketPtr requestPacket = getRequestPacket(req, streams[child]->get_Id());
   //streams[child]->send(1994, "%ld", 7);
   //delete requestPacket;
}

// Will have to modify when internal nodes can check assignments
double axMin, axMax, ayMin, ayMax;
bool ICareAbout(double x, double y) {
   if (axMin <= x && x < axMax && ayMin <= y && y < ayMax)
      return true;
   return false;
}
const char* PointFilter_format_string = "%d %lf %lf";
void PointFilter(std::vector< MRN::PacketPtr >& packets_in,
                 std::vector< MRN::PacketPtr >& packets_out,
                 std::vector< MRN::PacketPtr >&,
                 void**, MRN::PacketPtr&, const MRN::TopologyLocalInfo& t) {
   double x, y;
   int tick;
   int i;
   // Only push out packets containing points you care about!
   for(i = 0; i < packets_in.size(); i++) {
      packets_in[i]->unpack(PointFilter_format_string, &tick, &x, &y);
      if (ICareAbout(x, y)) {
         std::cout << "I care about (" << x << ", " << y;
         std::cout << ") because my assigned values are: " << axMin;
         std::cout << " through " << axMax << " and " << ayMin << " through ";
         std::cout << ayMax << "\n";
         packets_out.push_back(packets_in[i]);
      }
   }
}

const char * TreeInit_format_string = 
   "%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf";
void TreeInit(std::vector< MRN::PacketPtr >& packets_in,
              std::vector< MRN::PacketPtr >& packets_out,
              std::vector< MRN::PacketPtr >& /* packets_out_reverse */,
              void ** /* client data */,
		        MRN::PacketPtr& /* params */,
              const MRN::TopologyLocalInfo& t)
{
   const MRN::Network* net = t.get_Network();
   network = net;
   MRN::NetworkTopology* nTop = net->get_NetworkTopology();

   MRN::NetworkTopology::Node* thisNode = nTop->find_Node(t.get_Rank());
   const std::set<MRN::NetworkTopology::Node*> children = thisNode->get_Children();
   std::set<MRN::NetworkTopology::Node*>::iterator cIt = children.begin();
  
   int r;
   double minPoints, eps;
   double decay, delthresh;
   double xMin, xMax, yMin, yMax;
   double axMin, axMax, ayMin, ayMax;
   packets_in[0]->unpack(TreeInit_format_string,
                         &eps,
                         &minPoints,
                         &decay,
                         &delthresh,
                         &xMin, &xMax,
                         &yMin, &yMax,
                         &r,
                         &axMin, &axMax,
                         &ayMin, &ayMax);

   int nChildren = thisNode->get_NumChildren();
   
   // If message isn't intended for this node, ignore it!
   if (r == (int) t.get_Rank()){
      printf("Downstream filter from rank = %d\n", (int)t.get_Rank());
      printf("BOUNDS: X: %lf to %lf Y: %lf to %lf\n", xMin, xMax, yMin, yMax);
      printf("Assigned: X: %lf to %lf Y: %lf to %lf\n", axMin, axMax, 
         ayMin, ayMax);
      // If this node is a parent, send assignments to children
      if (net->is_LocalNodeParent()) {
         printf("\tIs a Parent\n");
         // Make a packet for each child
         double mins[2], maxes[2], aMins[2], aMaxes[2];
         
         mins[0] = xMin;
         mins[1] = yMin;
         maxes[0] = xMax;
         maxes[1] = yMax;
         aMins[0] = axMin;
         aMins[1] = ayMin;
         aMaxes[0] = axMax;
         aMaxes[1] = ayMax;

         double caMins[nChildren * 2], caMaxes[nChildren * 2];
         std::cout << &caMins[0] << " " << &caMaxes[0] << "\n";
         std::cout << "CREATING INTERNAL_NODE " << nChildren << "\n";
         internalNode = new InternalNode(2, nChildren, eps, minPoints, delthresh, decay, &mins[0], &maxes[0], &aMins[0], &aMaxes[0], &fakeReqFunc, &fakeResFunc, &caMins[0], &caMaxes[0]);
         std::cout << "Internal Node created! " << caMins[6] << "\n";
         unsigned int cnum = 0;
         for (; cIt != children.end(); cIt++) {
            // Assign placeholder values until InternalNode's assignment
            // code is done.
            int cr = ((int)(*cIt)->get_Rank());
            std::cout << "\tHAS CHILD: " << cr << "\n";
            std::cout << "Assigning " << caMins[cnum * 2 + 0] << " to " << caMaxes[cnum * 2 + 0] << " X\n";
            std::cout << "Assigning " << caMins[cnum * 2 + 1] << " to " << caMaxes[cnum * 2 + 1] << " Y\n";

            
            // This is quite a packet!
            std::cout << "Packing for child with rank = " << cr << " " << sizeof(cr) << "\n";
            MRN::PacketPtr new_packet(new MRN::Packet(packets_in[0]->get_StreamId(),
               packets_in[0]->get_Tag(),TreeInit_format_string, eps, minPoints,
               decay, delthresh, xMin, xMax, yMin, yMax, cr, caMins[cnum * 2 + 0], caMaxes[cnum * 2 + 0],
               caMins[cnum * 2 + 1], caMaxes[cnum * 2 + 1]));
            packets_out.push_back(new_packet);
            cnum++;
         }
      }
      if (net->is_LocalNodeBackEnd()) {
         std::cout << "\tBACKEND RANK = " << r << "\n";
         isLeaf = true;
         // Just send whatever we got to the backend code
         std::vector<MRN::PacketPtr>::iterator it = packets_in.begin();
         packets_out.push_back(*it);
      }
       
   }
}

const char* doNothing_format_string = "";
void doNothing( std::vector< MRN::PacketPtr >& packets_in,
                std::vector< MRN::PacketPtr >& packets_out,
                std::vector< MRN::PacketPtr >& /* packets_out_reverse */,
                void ** /* client data */, MRN::PacketPtr& /* params */,
                const MRN::TopologyLocalInfo& ) {
   std::cout << "doNothing was called\n";
   for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        packets_out.push_back( packets_in[i]);
    }
}

} /* extern "C" */
