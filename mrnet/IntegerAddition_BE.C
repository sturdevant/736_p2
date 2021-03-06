#include <unistd.h>
#include "mrnet/MRNet.h"
#include "IntegerAddition.h"
#include "LeafNode.h"
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace MRN;
   
Stream * g_stream = NULL;
Stream* responseStream = NULL;
unsigned int myRank;
char hostname[1024];

ReturnCode requestFunc(unsigned int, Request* req) {
   double w, nw;
   unsigned long id, type, time, l1, l2;
   double* val = (double*)malloc(2 * sizeof(double));
   req->unpack(&id, &type, &time, val, &w, &nw, &l1, &l2);
   //std::cout << "Request func called (type = " << type << ")!\n";
   MRN::PacketPtr new_packet(
      new MRN::Packet(
         g_stream->get_Id(),
         PROT_REQUEST,
         REQUEST_FORMAT_STRING,
         id, type, time, val[0], val[1], w, nw, l1, l2
      )
   );
   g_stream->send(new_packet);
   free(val);
}

ReturnCode responseFunc(Response* res) {
   unsigned long type, id, time, clustId;
   double nw;
   res->unpack(&id, &type, &time, &nw, &clustId);

   //std::cout << "Response func called (type = " << type << ")!\n";
   MRN::PacketPtr new_packet(
      new MRN::Packet(
         responseStream->get_Id(),
         PROT_RESPONSE,
         RESPONSE_FORMAT_STRING,
         id, type, time, nw, clustId, myRank
      )
   );
   responseStream->send(new_packet);
}

void* timerThreadStart(void* args) {
   struct timespec t1, t2;
   struct rusage r1, r2;
   clock_gettime(CLOCK_REALTIME, &t1);
   getrusage(RUSAGE_SELF, &r1);
   double pUsed = 0;
   while (1) {

      sleep(1);
      clock_gettime(CLOCK_REALTIME, &t2);
      getrusage(RUSAGE_SELF, &r2);

      double deltaT = (t2.tv_sec  - t1.tv_sec ) * 1000000.0 +
                      (t2.tv_nsec - t1.tv_nsec) / 1000.0;

      double u1 = (r1.ru_utime.tv_sec  + r1.ru_stime.tv_sec ) * 1000000.0 +
                  (r1.ru_utime.tv_usec + r1.ru_stime.tv_usec);

      double u2 = (r2.ru_utime.tv_sec  + r2.ru_stime.tv_sec ) * 1000000.0 +
                  (r2.ru_utime.tv_usec + r2.ru_stime.tv_usec);
      
      double deltaU = u2 - u1;

      pUsed = (pUsed + deltaU / deltaT) / 2;

      std::cout << "(" << myRank << ") CPU Usage = " << pUsed  << "\n";

      if (pUsed > 0.80) {
         Request r(REQUEST_TYPE_SLOWDOWN);
         requestFunc(0, &r);
         std::cout << "(" << myRank << ") REQUESTING SLOWDOWN\n";
      } else if (pUsed > 0.60) {
         Request r(REQUEST_TYPE_SPEED_WARNING);
         requestFunc(0, &r);
         std::cout << "(" << myRank << ") SPEED WARNING SENT\n";
      }

      memcpy(&t1, &t2, sizeof(struct timespec));
      memcpy(&r1, &r2, sizeof(struct rusage));
   }
}

int main(int argc, char **argv)
{
   hostname[1023] = '\0';
   gethostname(hostname, 1023);
   std::cout << "Backend started on: " << hostname << "\n";
   Stream* stream;
   PacketPtr p;
   int rc, tag=0, recv_val=0, num_iters=0;
   double eps, minPoints, decay, delthresh;
   double xMin, xMax, yMin, yMax;
   double axMin, axMax, ayMin, ayMax;
   double x, y, w, nw;
   unsigned long id, type, time, l1, l2;
   int r;
   Network * net = Network::CreateNetworkBE( argc, argv );
   myRank = net->get_LocalRank();
   responseStream = net->get_Stream(net->get_LocalRank());
   LeafNode* thisNode;

   pthread_t timeThread;
   
   pthread_create(&timeThread, 
                  NULL,
                  &timerThreadStart,
                  NULL);
   //*/
   unsigned int responseStreamId;
   double* ptArr = new double[2];
   Point* pt;
   Response* res;
   Request* req;
   struct timespec t1, t2;
   clock_gettime(CLOCK_REALTIME, &t2);
   struct timespec lastGoodTime;
   lastGoodTime.tv_sec = t2.tv_sec;
   lastGoodTime.tv_nsec = t2.tv_nsec;
   
   do {
         
      clock_gettime(CLOCK_REALTIME, &t1);
      rc = net->recv(&tag, p, &stream);
      clock_gettime(CLOCK_REALTIME, &t2);
      unsigned long recTime = t2.tv_nsec - t1.tv_nsec + 1000000000L * (t2.tv_sec - t1.tv_sec);
      if (recTime > 200000) {
         lastGoodTime.tv_sec = t2.tv_sec;
         lastGoodTime.tv_nsec = t2.tv_nsec;
      }

      unsigned long timeSinceGood = t2.tv_nsec - lastGoodTime.tv_nsec + 1000000000L * (t2.tv_sec - lastGoodTime.tv_sec);
      if (timeSinceGood > 1000000000L) {
         //std::cout << "BE SENDING SLOWDOWN PACKET!\n";
         fflush(stdout);
         //Request r(REQUEST_TYPE_SLOWDOWN);
         //requestFunc(0, &r);
         lastGoodTime.tv_sec = t2.tv_sec;
         lastGoodTime.tv_nsec = t2.tv_nsec;
      }

        if( rc == -1 ) {
            std::cerr << "BE: Network::recv() failure on " << hostname << "\n";
            fflush(stderr);
            exit(1);
            break;
        }
        else if( rc == 0 ) {

            // a stream was closed
            continue;
        }

        switch(tag) {

        case PROT_INIT:
            p->unpack("%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf %d",
               &eps, &minPoints, &decay, &delthresh, &xMin, &xMax, &yMin,
               &yMax, &r, &axMin, &axMax, &ayMin, &ayMax, &responseStreamId);
            std::cout << "Host: " << hostname << "\n" << "(" << r << ")\tBackend got init packet!\n";
            fflush(stdout);
            g_stream = net->get_Stream(responseStreamId);
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
               &requestFunc, &responseFunc
            );
            std::cout << "(" << r << ")\t Backend created leafnode!\n";
            fflush(stdout);
            stream->send( p);
            break;
        /*case PROT_STREAM:
            /*
            p->unpack("%d %lf %lf", &time, &ptArr[0], &ptArr[1]);
            pt = new Point(ptArr, 1, time);
            req = new Request(REQUEST_TYPE_ADD_POINT, pt, l1, l2);
            std::cout << "(" << r << ")\tGot a stream point!\n";
            thisNode->query(req, &res);//
            break;//*/
        case PROT_EXIT:
            fflush(stdout);
            if( stream->send(tag, "%d", 0) == -1 ) {
                fprintf( stderr, "BE: stream::send(%%s) failure in PROT_EXIT\n" );
                break;
            }
            if( stream->flush( ) == -1 ) {
                fprintf( stderr, "BE: stream::flush() failure in PROT_EXIT\n" );
            }
            break;//*/
        case PROT_REQUEST:
            p->unpack(
               REQUEST_FORMAT_STRING, 
               &id,
               &type,
               &time, 
               &ptArr[0], 
               &ptArr[1],
               &w,
               &nw,
               &l1,
               &l2
            );
            
            if (type == REQUEST_TYPE_SNAPSHOT) {
               if (stream->send(p
               /*MRN::PacketPtr(new MRN::Packet(
                  PROT_REQUEST, REQUEST_FORMAT_STRING,
                  id, type, time, ptArr[0], ptArr[1], w, nw, l1, l2))
                  */) == -1) {
                  std::cout << "ERROR SENDING SNAPSHOT REQUEST!\n";
               }
               if (stream->flush() == -1) {
                  std::cout << "ERROR FLUSHING SNAPSHOT REQUEST!\n";
               }//*/
            }
            pt = new Point(ptArr, 1, time);
            pt->setNWeight(nw);
            pt->setWeight(w);
            req = new Request((RequestType)type, pt, l1, l2);
            req->setID(id);
            //std::cout << "(" << r << ")\tGot a stream point!\n";
            //fflush(stdout);
            res = new Response((ResponseType)0, id);
            thisNode->query(req, res);
            delete pt;
            delete req;
            delete res;
            break;
        default:
            fprintf( stderr, "BE: Unknown Protocol: %d\n", tag );
            tag = PROT_EXIT;
            break;
        }

        fflush(stderr);

    } while( tag != PROT_EXIT );


    if( stream != NULL ) {
        while(!stream->is_Closed())
            sleep(1);

        delete stream;
    }

    // FE delete of the net will cause us to exit, wait for it
    net->waitfor_ShutDown();
    delete net;

    return 0;
}
