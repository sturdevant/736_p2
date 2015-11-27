
#include "mrnet/MRNet.h"
#include "IntegerAddition.h"
#include "LeafNode.h"

using namespace MRN;

ReturnCode fakeReqFunc(unsigned int, Request*) {
   std::cout << "Request func called!\n";
}

ReturnCode fakeResFunc(Response* res) {
   std::cout << "Response func called!\n";
}

int main(int argc, char **argv)
{
    Stream * stream = NULL;
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
            std::cout << "Got Init packet!\n";
            p->unpack("%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf",
               &eps, &minPoints, &decay, &delthresh, &xMin, &xMax, &yMin,
               &yMax, &r, &axMin, &axMax, &ayMin, &ayMax);
            double mins[2], maxes[2], aMins[2], aMaxes[2];
            mins[0] = xMin;
            mins[1] = yMin;
            maxes[0] = xMax;
            maxes[1] = yMax;
            aMins[0] = axMin;
            aMins[1] = ayMin;
            aMaxes[0] = axMax;
            aMaxes[1] = ayMax;
            thisNode = new LeafNode(2, eps, r, delthresh, &mins[0], &maxes[0], &aMins[0], &aMaxes[0], &fakeReqFunc, &fakeResFunc);
            std::cout << "LeafNode created (r = " << r << ")!\n";
            char ptFilename[80], assignFilename[80];
            snprintf(ptFilename, 80, "node_points_%d", r);
            snprintf(assignFilename, 80, "node_assigns_%d", r);
            FILE* pFile;
            pFile = fopen(ptFilename, "w");
            FILE* aFile;
            aFile = fopen(assignFilename, "w");
            thisNode->snapshot(pFile, aFile);
            fclose(pFile);
            fclose(aFile);
            break;
        case PROT_PTREQ:
            p->unpack("%d %lf %lf", &time, &x, &y);
            std::cout << "Got a point request! Cool!\n";
            break;
        case PROT_STREAM:
            p->unpack("%d %lf %lf", &time, &x, &y);
            std::cout << "Got a stream point!\n";
            break;
        case PROT_SUM:
            p->unpack( "%d %d", &recv_val, &num_iters );

            // Send num_iters waves of integers
            for( int i=0; i<num_iters; i++ ) {
                fprintf( stdout, "BE: Sending wave %u ...\n", i );
                if( stream->send(tag, "%d", recv_val*i) == -1 ) {
                    fprintf( stderr, "BE: stream::send(%%d) failure in PROT_SUM\n" );
                    tag = PROT_EXIT;
                    break;
                }
                if( stream->flush() == -1 ) {
                    fprintf( stderr, "BE: stream::flush() failure in PROT_SUM\n" );
                    break;
                }
                fflush(stdout);
                sleep(2); // stagger sends
            }
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
