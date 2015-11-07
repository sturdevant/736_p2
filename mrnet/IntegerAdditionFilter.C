/****************************************************************************
 * Copyright © 2003-2015 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/
#include <stdio.h>
#include <vector>
#include "mrnet/MRNet.h"
#include "mrnet/Packet.h"
#include "mrnet/NetworkTopology.h"

using namespace MRN;

extern "C" {

bool isLeaf = false;
Stream** streams = NULL;

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

const char * IntegerInit_format_string = "%d%f%f";
void IntegerInit( std::vector< PacketPtr >& packets_in,
                 std::vector< PacketPtr >& packets_out,
                 std::vector< PacketPtr >& /* packets_out_reverse */,
                 void ** /* client data */,
		 PacketPtr& /* params */,
                 const TopologyLocalInfo& t)
{
   const Network* net = t.get_Network();
   NetworkTopology* nTop = net->get_NetworkTopology();

   printf("Downstream filter from rank = %d\n", (int)t.get_Rank());
   NetworkTopology::Node* n = nTop->find_Node(t.get_Rank());
   const std::set<NetworkTopology::Node*> children = n->get_Children();
   std::set<NetworkTopology::Node*>::iterator cIt = children.begin();

   int nChildren = n->get_NumChildren();
   streams = (Stream**)malloc(sizeof(Stream*) * nChildren);

   if (streams == NULL) {
      printf("ERROR: Could not allocate streams to children!\n");
   }

   for (; cIt != children.end(); cIt++) {
      printf("\tHAS CHILD: %d\n", (int)(*cIt)->get_Rank());
      //streams[i] = new Stream();
   }

   if (net->is_LocalNodeChild()) {
      printf("\tCHILD\n");
   }
   
   if (net->is_LocalNodeParent()) {
      printf("\tPARENT\n");
   }
    
   if (net->is_LocalNodeBackEnd()) {
      printf("\tBACKEND\n");
      isLeaf = true;
   }
    

   std::vector<PacketPtr>::iterator it = packets_in.begin();
   for (; it != packets_in.end(); it++) {
      packets_out.push_back(*it);
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
