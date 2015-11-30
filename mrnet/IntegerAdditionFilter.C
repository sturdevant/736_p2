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

int r;
bool isLeaf = false;
const MRN::Network* network = NULL;
MRN::Stream** streams = NULL;
MRN::Communicator** comms = NULL;
MRN::Rank* childRanks = NULL;
InternalNode* internalNode;

// Will have to modify when internal nodes can check assignments
const char* PointFilter_format_string = "%d %lf %lf";
void PointFilter(std::vector< MRN::PacketPtr >& packets_in,
                 std::vector< MRN::PacketPtr >& packets_out,
                 std::vector< MRN::PacketPtr >&,
                 void**, MRN::PacketPtr&, const MRN::TopologyLocalInfo& t) {

   double* ptArr = new double[2];
   int tick;
   int i;
   const MRN::Network* net = t.get_Network();

   if (net->is_LocalNodeBackEnd()) {
      for (i = 0; i < packets_in.size(); i++) {
         packets_out.push_back(packets_in[i]);
      }
      return;
   }

   // Only push out packets containing points you care about!
   for(i = 0; i < packets_in.size(); i++) {
      packets_in[i]->unpack(PointFilter_format_string, &tick, &ptArr[0], &ptArr[1]);
      Point::Point* pt = new Point(ptArr, 1, tick);
      if (internalNode->admitPoint(pt)) {
         std::cout << "(" << net->get_LocalRank() << ") I care about (" << ptArr[0] << ", " << ptArr[1] << ")\n";
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
  
   //int r;
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
   if (r == (int)t.get_Rank()){
      //std::cout << "(" << r << ") Internal node got an init packet (Parent = " << net->is_LocalNodeParent() << ", Backend = " << net->is_LocalNodeBackEnd() << ")!\n";
      // If this node is a parent, send assignments to children
      if (net->is_LocalNodeParent()) {
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

         internalNode = new InternalNode(
            2, 
            nChildren, 
            eps, 
            minPoints, 
            delthresh, 
            decay, 
            1, 
            &mins[0], &maxes[0], 
            &aMins[0], &aMaxes[0], 
            &fakeReqFunc, &fakeResFunc, 
            &caMins[0], &caMaxes[0]
         );
         
         char assignFilename[80];
         snprintf(assignFilename, 80, "internal_assigns_%d", r);
         FILE* aFile;
         aFile = fopen(assignFilename, "w");
         internalNode->snapshot(aFile);
         fclose(aFile);
         
         unsigned int cnum = 0;
         for (; cIt != children.end(); cIt++) {
            // Assign placeholder values until InternalNode's assignment
            // code is done.
            int cr = ((int)(*cIt)->get_Rank());
            
            // This is quite a packet!
            MRN::PacketPtr new_packet(
               new MRN::Packet(packets_in[0]->get_StreamId(),
               packets_in[0]->get_Tag(),
               TreeInit_format_string, 
               eps, 
               minPoints,
               decay, 
               delthresh, 
               xMin, xMax, yMin, yMax, 
               cr, 
               caMins[cnum * 2 + 0], caMaxes[cnum * 2 + 0],
               caMins[cnum * 2 + 1], caMaxes[cnum * 2 + 1])
            );

            packets_out.push_back(new_packet);
            cnum++;
         }
      }

      if (net->is_LocalNodeBackEnd()) {
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
        //doNothing_format_string = packets_in[i]->get_FormatString();
        //std::cout << "Format string: " << doNothing_format_string << "\n";
        packets_out.push_back( packets_in[i]);
    }
}

} /* extern "C" */
