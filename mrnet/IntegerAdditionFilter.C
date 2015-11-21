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

using namespace MRN;

extern "C" {

bool isLeaf = false;
const Network* network = NULL;
Stream** streams = NULL;
Communicator** comms = NULL;
Rank* childRanks = NULL;
InternalNode * thisNode;
const char* PointMux_format_string = "%f%f";
void PointMux(std::vector< PacketPtr >& packets_in,
              std::vector< PacketPtr >& packets_out,
              std::vector< PacketPtr >& /* packets_out_reverse */,
              void ** /* client data */,
		        PacketPtr& /* params */,
              const TopologyLocalInfo& t)
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

const char * TreeInit_format_string = 
   "%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf";
void TreeInit(std::vector< PacketPtr >& packets_in,
              std::vector< PacketPtr >& packets_out,
              std::vector< PacketPtr >& /* packets_out_reverse */,
              void ** /* client data */,
		        PacketPtr& /* params */,
              const TopologyLocalInfo& t)
{
   const Network* net = t.get_Network();
   network = net;
   NetworkTopology* nTop = net->get_NetworkTopology();

   NetworkTopology::Node* thisNode = nTop->find_Node(t.get_Rank());
   const std::set<NetworkTopology::Node*> children = thisNode->get_Children();
   std::set<NetworkTopology::Node*>::iterator cIt = children.begin();
  
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

   /* Old packet format, don't think we need filters...
   packets_in[0]->unpack(TreeInit_format_string,
                         &upTFilter,
                         &upSFilter,
                         &downTFilter,
                         &xMin, &xMax,
                         &yMin, &yMax,
                         &r,
                         &axMin, &axMax,
                         &ayMin, &ayMax);
   */
   int nChildren = thisNode->get_NumChildren();
   
   /* Don't think we need to create streams/communicators in filter...
   streams = (Stream**)malloc(sizeof(Stream*) * nChildren);
   comms = (Communicator**)malloc(sizeof(Communicator*) * nChildren);
   if (streams == NULL || comms == NULL) {
      printf("ERROR: Could not allocate streams and communicators!\n");
   }
   */


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
         for (; cIt != children.end(); cIt++) {
            // Assign placeholder values until InternalNode's assignment
            // code is done.
            double caxMin, caxMax, cayMin, cayMax;
            int cr = ((int)(*cIt)->get_Rank());
            caxMin = -(double)cr;
            caxMax = (double)cr;
            cayMin = -(double)cr;
            cayMax = (double)cr;
            printf("\tHAS CHILD: %d\n", cr);
            
            // This is quite a packet!
            PacketPtr new_packet(new Packet(packets_in[0]->get_StreamId(),
               packets_in[0]->get_Tag(),TreeInit_format_string, eps, minPoints,
               decay, delthresh, xMin, xMax, yMin, yMax, cr, caxMin, caxMax,
               cayMin, cayMax));
            packets_out.push_back(new_packet);
         }
      }
      if (net->is_LocalNodeBackEnd()) {
         printf("\tBACKEND\n");
         isLeaf = true;
         // Just send whatever we got to the backend code
         std::vector<PacketPtr>::iterator it = packets_in.begin();
         packets_out.push_back(*it);
      }
       
   }
}

const char * IntegerAdd_format_string = "%d";
void IntegerAdd( std::vector< PacketPtr >& packets_in,
                 std::vector< PacketPtr >& packets_out,
                 std::vector< PacketPtr >& /* packets_out_reverse */,
                 void ** /* client data */,
		 PacketPtr& /* params */,
                 const TopologyLocalInfo& )
{
   std::cout << "IntegerAdd was called\n";
    int sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        PacketPtr cur_packet = packets_in[i];
	int val;
	cur_packet->unpack("%d", &val);
        sum += val;
    }
    
    PacketPtr new_packet ( new Packet(packets_in[0]->get_StreamId(),
                                      packets_in[0]->get_Tag(), "%d", sum ) );
    packets_out.push_back( new_packet );
}

} /* extern "C" */
