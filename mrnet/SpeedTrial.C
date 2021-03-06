#include "logger.h"
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include "mrnet/MRNet.h"
#include "Response.h"
#include "Request.h"
#include "IntegerAddition.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

using namespace std;
using namespace MRN;

#define ONE_HOUR 3600000

#define N_DIMENSIONS 2
#define DIM_BUFFER_SIZE 512
#define FIRST_TICK 1446841305000//1448997780000
#define LAST_TICK  FIRST_TICK + 60 * ONE_HOUR

#define END_TIME 1800.0l

#define MAX_NODES 64
#define TICK_INCREASE_START 230
#define TICK_INCREASE_FACTOR (ONE_HOUR / 36000.0l)
#define TICK_REDUCTION_FACTOR (TICK_INCREASE_FACTOR)

#define MAX_RT 2000000000
#define LOW_RT 250000000

#define N_LEAF_NODES 4
unsigned long responseCount[MAX_NODES];
double totalNodeResponseTime[MAX_NODES];
                   
Network * net;
Communicator* comm_BC; 
Stream* stream_Stream;
Stream* snapshot_Stream;
TimedQueue responseQueue(sizeof(unsigned long), 2000000);

void* startPointQueryThread(void* filename);
void* startStreamThread(void* filename);

// These globals will be set before any threads and are read-only, making them
// safe for reading in each thread.
double ticksPerSec = ONE_HOUR / 80;//150000.0;
int logging;

unsigned long tick = FIRST_TICK;
double totalTime = 0;
unsigned long serverTime = 0;
unsigned long totalFrontEndRecs = 0;
bool speedWarning = false;

double snapshotInterval = ticksPerSec;//ONE_HOUR;//150000.0;
double nextSnapshotTime = FIRST_TICK + snapshotInterval;
double nextStreamTime = FIRST_TICK + ticksPerSec;

double eps = 0.50;
double minPts = 50.0;
double decayFactor = 0.99;
double delthresh = 0.1;
double xMin = -90;//24;
double xMax = 90;//50;
double yMin = -180;//-125;
double yMax = 180;//-66;

unsigned long nSent = 0;
unsigned long nBack = 0;

unsigned long totalResponses = 0;
double totalResponseTime = 0;
unsigned long queriesSent = 0;
unsigned long pointsSent = 0;
unsigned long replaysSent = 0;

pthread_mutex_t timeLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t timeCond = PTHREAD_COND_INITIALIZER;

typedef struct threadargs {
   void* input;
   void* output;
} threadargs;

unsigned long getNewRequestId() {
   static unsigned long nextReqId = 0;
   nextReqId++;
   return nextReqId;
}

int changeSpeed() {
   unsigned int max = 0;
   double maxRT = 0;
   /*for (int i = 0; i < MAX_NODES; i++) {
      if (responseCount[i] != 0) {
         double rt = totalNodeResponseTime[i] / responseCount[i];
         if (rt > maxRT) {
            maxRT = rt;
            max = i;
         }
      }
      responseCount[i] = 0;
      totalNodeResponseTime[i] = 0;
   }//*/
   if (totalResponses != 0) {
      maxRT = totalResponseTime / totalResponses;
   }
   
   if (maxRT > MAX_RT || responseQueue.getCount() > 1000) {
      return -1;
   }

   if (maxRT != 0 && maxRT < LOW_RT && responseQueue.getCount() < 1000) {
      return 1;
   }

   return 0;
}

void frontEndSnapshotRequest() {
   //std::cout << "Sending snapshot!\n";
   unsigned long id = getNewRequestId();
   for (unsigned int i = 0; i < N_LEAF_NODES; i++) {
      //nSent++;
      //responseQueue.add(&id);
   }
   //std::cout << "Added to queue!\n";
   MRN::PacketPtr packet(
      new MRN::Packet(snapshot_Stream->get_Id(),
         PROT_REQUEST, REQUEST_FORMAT_STRING, 
         id, REQUEST_TYPE_SNAPSHOT, serverTime, 
         1.0, 1.0, 1.0, 1.0, 1, 1
      )
  );

   //std::cout << "Created packet!\n";
   if (snapshot_Stream->send(packet) == -1) {
      std::cout << "Error in sending snapshot request\n";
   }

   //std::cout << "About to flush snapshot request!\n";
   
   if (snapshot_Stream->flush() == -1)
      ;//std::cout << "Error in flushing stream\n";


   //*/
}

void frontEndPointRequest(double* pt) {
   //std::cout << "Read point to request at time " << tick << " X = " << pt[0] << " Y = " << pt[1] << std::endl;
   unsigned long id = getNewRequestId();

   pthread_mutex_lock(&timeLock);
   queriesSent++;
   nSent++;
   pthread_mutex_unlock(&timeLock);

   unsigned long* idPtr = &id;
   responseQueue.add(idPtr);
   if (stream_Stream->send(
         PROT_REQUEST, REQUEST_FORMAT_STRING, 
         id, REQUEST_TYPE_POINT_DATA, serverTime, pt[0], pt[1], 1.0, 0.0, 0, 0
       ) 
       == -1) {
      std::cout << "Error in sending point request\n";
   }
   if (stream_Stream->flush() == -1)
      ;//std::cout << "Error in flushing stream\n";
}

void frontEndPointStream(double* pt) {
   unsigned long id = getNewRequestId();
   //std::cout << "Read point to stream at time " << tick << " X = " << pt[0] << " Y = " << pt[1] << " ID = " << id << std::endl;

   pthread_mutex_lock(&timeLock);
   pointsSent++;
   pthread_mutex_unlock(&timeLock);


   if (stream_Stream->send(
         PROT_REQUEST, REQUEST_FORMAT_STRING, 
         id, REQUEST_TYPE_ADD_POINT, serverTime, pt[0], pt[1], 1.0, 0.0, 0, 0
       ) 
       == -1) {
      std::cout << "Error in sending stream point\n";
   }
   if (stream_Stream->flush() == -1)
      std::cout << "Error in flushing stream\n";
}

bool saw_failure = false;
void Failure_Callback(Event* evt, void*) {
   if( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
       (evt->get_Type() == TopologyEvent::TOPOL_REMOVE_NODE) )
       saw_failure = true;
}

void* listenStart(void* listenArgs) {
   int rc, tag;
   Stream* stream;
   PacketPtr p;
   double x, y, w, nw;
   unsigned long id, type, time, l1, l2, clustId, cr;
   std::cout << "LISTENER PID = " << syscall(SYS_gettid) << "\n";
   struct timespec t1, t2;
   clock_gettime(CLOCK_REALTIME, &t2);
   struct timespec lastGoodTime;
   lastGoodTime.tv_sec = t2.tv_sec;
   lastGoodTime.tv_nsec = t2.tv_nsec;
   
   
   
   while(1) {

      clock_gettime(CLOCK_REALTIME, &t1);
      
      rc = net->recv(&tag, p, &stream);
      clock_gettime(CLOCK_REALTIME, &t2);
      unsigned long recTime = t2.tv_nsec - t1.tv_nsec + 1000000000L * (t2.tv_sec - t1.tv_sec);
      if (recTime > 100000) {
         lastGoodTime.tv_sec = t2.tv_sec;
         lastGoodTime.tv_nsec = t2.tv_nsec;
      }

      unsigned long timeSinceGood = t2.tv_nsec - lastGoodTime.tv_nsec + 1000000000L * (t2.tv_sec - lastGoodTime.tv_sec);
      if (timeSinceGood > 1000000000L) {
         if (ticksPerSec > TICK_REDUCTION_FACTOR * 2) {
            std::cout << "FRONT END SLOW DOWN!\n";
            ticksPerSec -= TICK_REDUCTION_FACTOR;
         }
         lastGoodTime.tv_sec = t2.tv_sec;
         lastGoodTime.tv_nsec = t2.tv_nsec;
      }
      
      
      if (rc < 0) 
         std::cout << "Error receiving in frontend listenStart\n";
      else {
         totalFrontEndRecs++;
         //std::cout << "Successfully received in frontend\n";
         if (tag == PROT_REQUEST) {
               
            p->unpack(
               REQUEST_FORMAT_STRING, 
               &id, &type, &time, &x, &y, &w, &nw, &l1, &l2
            );

            if (type == REQUEST_TYPE_SLOWDOWN) {
               if (ticksPerSec > TICK_REDUCTION_FACTOR * 2) {
                  ticksPerSec -= TICK_REDUCTION_FACTOR;
               }
               snapshotInterval *= TICK_REDUCTION_FACTOR;
               std::cout << "RECEIVED SLOW DOWN PACKET!\n";
               continue;
            } else if (type == REQUEST_TYPE_SPEED_WARNING) {
               speedWarning = true;
               continue;
            }

            if (type == REQUEST_TYPE_SNAPSHOT) {
               //std::cout << "Got snapshot response!\n";
               try {
                  //nBack += N_LEAF_NODES;
                  pthread_mutex_lock(&timeLock);
                  //double rt = //responseQueue.remove(&id);
                  //totalResponseTime += rt;
                  //totalResponses++;
                  pthread_mutex_unlock(&timeLock);
               } catch (const char* c) {
                  std::cout << c;
               }
               continue;
            }

            pthread_mutex_lock(&timeLock);
            replaysSent++;
            pthread_mutex_unlock(&timeLock);
                  

            MRN::PacketPtr pOut(
               new MRN::Packet( 
                  stream->get_Id(), 
                  PROT_REQUEST, REQUEST_FORMAT_STRING, 
                  id, type, time, x, y, w, nw, l1, l2 
               )
            );
            stream->send(pOut);
         } else if (tag == PROT_RESPONSE) {
            p->unpack(
               RESPONSE_FORMAT_STRING,
               &id, &type, &time, &nw, &clustId, &cr
            );
            nBack++;
            try {
               pthread_mutex_lock(&timeLock);
               double rt = responseQueue.remove(&id);
               totalResponseTime += rt;
               totalResponses++;
               responseCount[cr]++;
               totalNodeResponseTime[cr] += rt;
               pthread_mutex_unlock(&timeLock);
            } catch (const char* c) {
               std::cout << c;
            }//
         }
      }
   }
   return NULL;
}

// Initialize mrnet w/ specified parameters
void initMRNet(double eps, 
               double minPoints, 
               double decay, 
               double delthresh,
               double xMin,
               double xMax, 
               double yMin, 
               double yMax) {
   int retval;
   const char* topology_file = 
      "/u/s/t/sturdeva/public/736/736_p2/mrnet/topologies/ntop.2";
   const char* backend_exe = 
      "/u/s/t/sturdeva/public/736/736_p2/mrnet/IntegerAddition_BE";
   const char* so_file = 
      "/u/s/t/sturdeva/public/736/736_p2/mrnet/IntegerAdditionFilter.so";
   
   const char* dummy_argv = NULL;

   net = Network::CreateNetworkFE(topology_file, backend_exe, &dummy_argv);

   if (net->has_Error()) {
      net->perror("Network creation failed\n");
      exit(-1);
   }
   if (!net->set_FailureRecovery(false)) {
      fprintf(stdout, "Failed to disable failure recovery\n");
      delete net;
      exit(-1);
   }
   bool cbrett = net->register_EventCallback(Event::TOPOLOGY_EVENT,
      TopologyEvent::TOPOL_REMOVE_NODE, Failure_Callback, NULL);
   if (cbrett == false) {
      printf("Failed to register callback for node failure event\n");
      delete net;
      exit(-1);
   }

   vector<const char*> filterNames;
   filterNames.push_back("doNothing");
   filterNames.push_back("TreeInit");
   filterNames.push_back("PointFilter");
   vector<int> filterIds;
   
   // Make sure path to "so_file" is in LD_LIBRARY_PATH
   retval = net->load_FilterFuncs(so_file, filterNames, filterIds);
   if (retval == -1) {
      fprintf(stderr, "Network::load_FilterFunc() failure\n");
      delete net;
      exit(-1);
   }

   double axMin = xMin;
   double axMax = xMax;
   double ayMin = yMin;
   double ayMax = yMax;

   std::cout << "X: " << axMin << " to " << axMax << "\n";
   std::cout << "Y: " << ayMin << " to " << ayMax << "\n";
   comm_BC = net->get_BroadcastCommunicator();

   snapshot_Stream = net->new_Stream(comm_BC, 
                                   TFILTER_NULL, 
                                   SFILTER_WAITFORALL,
                                   TFILTER_NULL);

   stream_Stream = net->new_Stream(comm_BC, 
                                   filterIds[0], 
                                   SFILTER_DONTWAIT,
                                   filterIds[2]);

   Stream* init_Stream = net->new_Stream(comm_BC, 
                                         filterIds[0],
                                         SFILTER_WAITFORALL, 
                                         filterIds[1]);

   int r = net->get_NetworkTopology()->get_Root()->get_Rank();
   int tag = PROT_INIT;
   cout << "sending initialization packet!\n";
   if (init_Stream->send(tag,
       "%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf %d", eps, minPoints, 
       decay, delthresh, xMin, xMax, yMin, yMax, r, xMin, xMax, yMin, yMax, 
       stream_Stream->get_Id())
       == -1 ) {
      fprintf(stderr, "stream::send() failure\n");
      exit(-1);
   }
   
   cout << "flushing all packets!\n";
   if (init_Stream->flush() == -1) {
      fprintf(stderr, "stream::flush() failure\n");
      exit(-1);
   }
   PacketPtr p;
   if (init_Stream->recv(&tag, p) == -1) {
      std::cout << "ERROR: Did not receive init ack!\n";
   }
   //cout << "deleting initialization stream!\n";
   //delete init_Stream;
}


int main(int argc, char** argv) {
   if (argc < 3) {
      printf("usage: client {[<service type> <filename>], [...]...}\n");
      printf("service types:\n \t\"-point\" query points\n");
      printf("\t\"-stream\" stream, a source of data points\n");
      exit(-1);
   }
   
   std::cout << "PARENT PID = " << syscall(SYS_gettid) << "\n";

   initMRNet(eps, minPts, decayFactor, delthresh, xMin, xMax, yMin, yMax);
   pthread_t* listenThread = (pthread_t*)malloc(sizeof(pthread_t));
   int retval = pthread_create(listenThread, 
                               NULL, 
                               &listenStart, 
                               NULL);
   int i = 1;
   int threadCount = 0;

   // Allocate an array equal to the largest number of threads possible.
   pthread_t* threads = (pthread_t*)malloc((argc / 2) * sizeof(pthread_t));
   threadargs* tArgs = (threadargs*)malloc((argc / 2) * sizeof(threadargs));
   
   void* (*entry)(void*);
   while (i < argc) {
      if (!strcmp(argv[i], "-point")) {

         // Create thread to handle point queries
         entry = &startPointQueryThread;
         tArgs[threadCount].input = argv[i + 1];
         tArgs[threadCount].output = NULL;

      } else if (!strcmp(argv[i], "-stream")) {

         // Create thread to provide a stream of points to the server
         entry = &startStreamThread;
         tArgs[threadCount].input = argv[i + 1];
         tArgs[threadCount].output = NULL;
      
      }

      retval = pthread_create(&threads[threadCount], 
                              NULL, 
                              entry, 
                              &tArgs[threadCount]);

      // If a thread fails, we'll abort client operation completely.
      if (retval) {
         printf("ERROR: Thread creation %d failed! Exiting...\n", i);
         exit(-1);
      }
      threadCount++;
      i += 2;

   }

   FILE* rtFile = fopen("/u/s/t/sturdeva/public/736/736_p2/mrnet/PaperData/1x4eps/response_times.csv", "w");

   struct timespec sleepTime;
   sleepTime.tv_nsec = 50000000;
   sleepTime.tv_sec = 0;
   double nextStreamTime = 1.0;
   double nextSnapshotTime = 1.0;

   // TODO: Timekeeping/cv_signal
   while (tick < LAST_TICK && totalTime < END_TIME) {
      nanosleep(&sleepTime, NULL);
      
      pthread_mutex_lock(&timeLock);
      tick += ticksPerSec * ((double)sleepTime.tv_nsec) / 1000000000.0l;
      totalTime += ((double)sleepTime.tv_nsec) / 1000000000.0l;
      pthread_cond_broadcast(&timeCond);
      pthread_mutex_unlock(&timeLock);

      int spdChange = changeSpeed();
      if (totalTime >= nextStreamTime) {
         fprintf(rtFile, "%ld,%ld,%lf,%ld,%ld,%ld,%lf,%ld\n",
            serverTime,
            tick, 
            totalResponseTime / totalResponses, 
            queriesSent, 
            replaysSent, 
            pointsSent,
            ticksPerSec,
            totalFrontEndRecs
         );
         fflush(rtFile);
         
         totalResponseTime = 0;
         totalResponses = 0;
         queriesSent = 0;
         replaysSent = 0;
         pointsSent = 0;
         totalFrontEndRecs = 0;

         fclose(rtFile);
         rtFile = fopen("response_times.csv", "a");
         std::cout << "Time = " << serverTime << " speed = " << ticksPerSec << "\n";
         std::cout << "Queue Length = " << responseQueue.getCount() << " (" << nSent - nBack << " " << nSent << " - " << nBack << ")\n";
         serverTime++;
         nextStreamTime += 1.0;//ticksPerSec;
      }
      
      if (totalTime >= nextSnapshotTime) {
         if (spdChange == -1) {
            ticksPerSec -= TICK_REDUCTION_FACTOR;
            std::cout << "Slowing down! New speed = " << ticksPerSec << " ticks/sec\n";
         } else if (spdChange == 1 && 
                    !speedWarning && 
                    totalTime > TICK_INCREASE_START) {
               //ticksPerSec += TICK_INCREASE_FACTOR;
         }
         frontEndSnapshotRequest();
         nextSnapshotTime += 1.0;//snapshotInterval;
         speedWarning = false;
      }
   }

   fclose(rtFile);

   std::cout << "Closed response time file!\n";

   // Wait for each of the child threads to finish.
   for (; threadCount > 0; threadCount--) {
      pthread_cancel(threads[threadCount - 1]);
      pthread_join(threads[threadCount - 1], NULL);
   }

   sleep(10);

   printf("Client finished. Final ticks/sec = %lf\n", ticksPerSec);
   return 0;
}

void* startPointQueryThread(void* arg) {

   std::cout << "Starting point query thread!\n";
   threadargs* args = (threadargs*)arg;
   int i;
   FILE* outfd;
   int fd = -1;//connectToHost();
   size_t index;
   std::cout << "QUERIER PID = " << syscall(SYS_gettid) << "\n";
   char* writeBuf = (char*)malloc(N_DIMENSIONS * sizeof(double));
   char* buf = (char*)malloc(N_DIMENSIONS * DIM_BUFFER_SIZE);
   if (writeBuf == NULL || buf == NULL) {
      printf("ERROR: Could not allocate line buffer! Exiting...\n");
      exit(-1);
   }

   std::cout << "Beginning query reading!\n";

   // Make 1 byte of room at the beginning for the type.
   double* pt = (double*)(&writeBuf[0]);
   std::fstream in((char*)args->input, fstream::in);

   TimedQueue* logQueue;
   pthread_t loggerThread;

   while(!in.eof()) {
      if (!in.getline(buf, N_DIMENSIONS * DIM_BUFFER_SIZE - 1)) {
         continue;
      }

      //std::cout << "Line read: " << buf << std::endl;
      if (buf[0] == 't') {
        
         // This is a timestamp, so we may need to wait.
         unsigned long t = atol(&buf[1]);
         //std::cout << "Timestamp found: " << t << std::endl;
         pthread_mutex_lock(&timeLock);
         while (t > tick) {
            // This timestamp hasn't come yet, so we need to go to sleep.
            pthread_cond_wait(&timeCond, &timeLock);

            //TODO: Remove if unnecessary.
            //Someone else might be waiting, so signal them too.
            //pthread_cond_signal(&timeCond);
         }
         pthread_mutex_unlock(&timeLock);

      } else {
         
         //index = 0;

         char* curBuf = buf;
         // This will be an actual point with components separated by commas.
         for (i = 0; i < N_DIMENSIONS; i++) {
            pt[i] = atof(curBuf);
            curBuf = strchr(curBuf, ',') + 1;
         }

         //printf("%02X\n", *writeBuf);
         //std::cout << "About to write to server:" << " X = " << pt[0] << " Y = " << pt[1] << std::endl;

         // Now, we need to put out the point in a format that the server can
         // iterpret properly (including tagging it as a point query).
         frontEndPointRequest(pt);
         //std::cout << "Sent data to server...\n";
      }
   } 
   std::cout << "Thread finished!\n";
}

void* startStreamThread(void* arg) {

   threadargs* args = (threadargs*)arg;
   int i;
   FILE* outfd;
   int fd = -1;//connectToHost();
   size_t index;
   std::cout << "STREAMER PID = " << syscall(SYS_gettid) << "\n";
   char* writeBuf = (char*)malloc(N_DIMENSIONS * sizeof(double));
   char* buf = (char*)malloc(N_DIMENSIONS * DIM_BUFFER_SIZE);
   if (writeBuf == NULL || buf == NULL) {
      printf("ERROR: Could not allocate line buffer! Exiting...\n");
      exit(-1);
   }

   // Make 1 byte of room at the beginning for the type.
   double* pt = (double*)(&writeBuf[0]);
   std::fstream in((char*)args->input, fstream::in);

   TimedQueue* logQueue;
   pthread_t loggerThread;

   while(!in.eof()) {
      if (!in.getline(buf, N_DIMENSIONS * DIM_BUFFER_SIZE - 1)) {
         continue;
      }

      if (buf[0] == 't') {
         // This is a timestamp, so we may need to wait.
         unsigned long t = atol(&buf[1]);
         //std::cout << "Timestamp found: " << t << std::endl;
         pthread_mutex_lock(&timeLock);
         while (t > tick) {
            // This timestamp hasn't come yet, so we need to go to sleep.
            pthread_cond_wait(&timeCond, &timeLock);
         }
         pthread_mutex_unlock(&timeLock);

      } else {
         char* curBuf = buf;
         // This will be an actual point with components separated by commas.
         for (i = 0; i < N_DIMENSIONS; i++) {
            pt[i] = atof(curBuf);
            curBuf = strchr(curBuf, ',') + 1;
         }

         frontEndPointStream(pt);
      }
   } 
   std::cout << "Thread finished!\n";
}

