
#include "mrnet/MRNet.h"
#include "IntegerAddition.h"
#include "LeafNode.h"

using namespace MRN;
   
Stream * g_stream = NULL;

ReturnCode fakeReqFunc(unsigned int, Request* req) {
   std::cout << "Request func called!\n";
   double w, nw;
   unsigned long id, type, time, l1, l2;
   double* val = (double*)malloc(2 * sizeof(double));
   req->unpack(&id, &type, &time, val, &w, &nw, &l1, &l2);
   MRN::PacketPtr new_packet(
      new MRN::Packet(
         g_stream->get_Id(),
         PROT_REQUEST,
         REQUEST_FORMAT_STRING,
         id,
         type,
         time,
         val[0],
         val[1],
         w,
         nw,
         l1,
         l2
      )
   );
   g_stream->send(new_packet);
   free(val);
}

ReturnCode fakeResFunc(Response* res) {
   std::cout << "Response func called!\n";
}

int main(int argc, char **argv)
{
   Stream* stream;
   PacketPtr p;
   int rc, tag=0, recv_val=0, num_iters=0;
   double eps, minPoints, decay, delthresh;
   double xMin, xMax, yMin, yMax;
   double axMin, axMax, ayMin, ayMax;
   double x, y;
   int time;
   int r;
   Network * net = Network::CreateNetworkBE( argc, argv );
   LeafNode* thisNode;

   unsigned long l1 = 0;
   unsigned long l2 = 0;
   double* ptArr = new double[2];
   Point* pt;
   Response res;
   Request* req;
    do {
        
        rc = net->recv(&tag, p, &stream);
        if( rc == -1 ) {
            fprintf( stderr, "BE: Network::recv() failure\n" );
            break;
        }
        else if( rc == 0 ) {

            // a stream was closed
            continue;
        }
        switch(tag) {

        case PROT_INIT:
            p->unpack("%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf",
               &eps, &minPoints, &decay, &delthresh, &xMin, &xMax, &yMin,
               &yMax, &r, &axMin, &axMax, &ayMin, &ayMax);
            //std::cout << "(" << r << ")\tBackend got init packet!\n";
            g_stream = stream;
            double mins[2], maxes[2], aMins[2], aMaxes[2];
            mins[0] = xMin;
            mins[1] = yMin;
            maxes[0] = xMax;
            maxes[1] = yMax;
            aMins[0] = axMin;
            aMins[1] = ayMin;
            aMaxes[0] = axMax;
            aMaxes[1] = ayMax;

            thisNode = new LeafNode(
               2, 
               eps, 
               minPoints, 
               decay, 
               1, 
               r, 
               delthresh, 
               &mins[0], &maxes[0], 
               &aMins[0], &aMaxes[0], 
               &fakeReqFunc, &fakeResFunc
            );

            //std::cout << "(" << r << ")\tLeaf Node created!\n";
            
            char ptFilename[80], assignFilename[80];
            snprintf(ptFilename, 80, "node_points_%d", r);
            snprintf(assignFilename, 80, "node_assigns_%d", r);
            FILE* pFile;
            pFile = fopen(ptFilename, "w");
            FILE* aFile;
            aFile = fopen(assignFilename, "w");
            thisNode->snapshot(pFile, aFile);
            fclose(pFile);
            fclose(aFile);//*/
            break;
        case PROT_PTREQ:
            p->unpack("%d %lf %lf", &time, &ptArr[0], &ptArr[1]);
            pt = new Point(ptArr, 1, time);
            req = new Request(REQUEST_TYPE_POINT_DATA, pt, l1, l2);
            std::cout << "Got a point request! Cool!\n";
            thisNode->query(req, &res);
            break;
        case PROT_STREAM:
            p->unpack("%d %lf %lf", &time, &ptArr[0], &ptArr[1]);
            pt = new Point(ptArr, 1, time);
            req = new Request(REQUEST_TYPE_ADD_POINT, pt, l1, l2);
            std::cout << "(" << r << ")\tGot a stream point!\n";
            thisNode->query(req, &res);
            break;
        case PROT_EXIT:
            if( stream->send(tag, "%d", 0) == -1 ) {
                fprintf( stderr, "BE: stream::send(%%s) failure in PROT_EXIT\n" );
                break;
            }
            if( stream->flush( ) == -1 ) {
                fprintf( stderr, "BE: stream::flush() failure in PROT_EXIT\n" );
            }
            break;
        case PROT_REQUEST:
            p->unpack("%d %lf %lf", &time, &ptArr[0], &ptArr[1]);
            pt = new Point(ptArr, 1, time);
            req = new Request(REQUEST_TYPE_ADD_POINT, pt, l1, l2);
            std::cout << "(" << r << ")\tGot a stream point!\n";
            thisNode->query(req, &res);
            break;
        default:
            fprintf( stderr, "BE: Unknown Protocol: %d\n", tag );
            tag = PROT_EXIT;
            break;
        }

        fflush(stderr);

    } while( tag != PROT_EXIT );

    if( stream != NULL ) {
        while( ! stream->is_Closed() )
            sleep(1);

        delete stream;
    }

    // FE delete of the net will cause us to exit, wait for it
    net->waitfor_ShutDown();
    delete net;

    return 0;
}
