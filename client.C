
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


using namespace std;

#define N_DIMENSIONS 2
#define DIM_BUFFER_SIZE 512

void* startPointQueryThread(void* filename);
void* startStreamThread(void* filename);
void* startPolygonQueryThread(void* filename);

// These globals will be set before any threads and are read-only, making them
// safe for reading in each thread.
struct hostent* host;
int portnum;
double ticksPerSec;
int log;
unsigned long tick;

pthread_mutex_t timeLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t timeCond = PTHREAD_COND_INITIALIZER;

typedef struct threadargs {
   void* input;
   void* output;
} threadargs;

int main(int argc, char** argv) {
   if (argc < 5) {
      printf("usage: client <hostname> <port> <timestep> [-log] {[<service type> <filename> [<logfilename>], [...]...}\n");
      printf("timestep is in units of ticks/second\n");
      printf("service types:\n \t\"-point\" query points\n");
      printf("\t\"-stream\" stream, a source of data points\n");
      printf("\t\"-polygon\" query for polygons\n");
      printf("If -log is specified, you must provide a log filename for \"-point\" and \"-polygon\" requests\n");
      exit(-1);
   }

   log = 0;

   host = gethostbyname(argv[1]);

   if (host == NULL) {
      printf("ERROR: Could not resolve host %s! Exiting...\n", argv[1]);
      exit(-1);
   }

   portnum = atoi(argv[2]);

   int i = 2;

   if (strcmp(argv[3], "-log")) {
      i++;
      log = 1;
   }

   int retval, threadCount = 0;

   // Allocate an array equal to the largest number of threads possible.
   pthread_t* threads = (pthread_t*)malloc((argc / 2 - 1) * sizeof(pthread_t));
   threadargs* tArgs = (threadargs*)malloc((argc / 2 - 1) * sizeof(threadargs));

   if (tArgs == NULL || threads == NULL) {
      printf("ERROR: Could not allocate memory for threads!\n");
      exit(-1);
   }

   void* (*entry)(void*);
   while (i < argc) {
      if (!strcmp(argv[i], "-point")) {

         // Create thread to handle point queries
         entry = &startPointQueryThread;
         tArgs[threadCount].input = argv[i + 1];
         if (log) {
            tArgs[threadCount].output = argv[i + 2];
            i += 3;
         } else {
            tArgs[threadCount].output = NULL;
            i += 2;
         }

      } else if (!strcmp(argv[i], "-stream")) {

         // Create thread to provide a stream of points to the server
         entry = &startStreamThread;
         tArgs[threadCount].input = argv[i + 1];
         tArgs[threadCount].output = NULL;
         i += 2;
         
      } else if (!strcmp(argv[i], "-polygon")) {

         // Create thread to handle polygon queries
         entry = &startPolygonQueryThread;
         tArgs[threadCount].input = argv[i + 1];
         if (log) {
            tArgs[threadCount].output = argv[i + 2];
            i += 3;
         } else {
            tArgs[threadCount].output = NULL;
            i += 2;
         }

      } else {

         // We don't really need the server to be all that tolerant of input
         // error since it is just a simulation of external calls. If a bad
         // service type is specified, just report it and exit.
         printf("ERROR: Invalid service type %s. Exiting...\n", argv[i]);
         exit(-1);
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

   }

   // TODO: Timekeeping/cv_signal
   while(1) {
      retval = sleep(1);
      if (retval != 0) {
         printf("ERROR: Failed to sleep!\n");
      }
      pthread_mutex_lock(&timeLock);
      tick++;
      pthread_cond_broadcast(&timeCond);
      pthread_mutex_unlock(&timeLock);
   }

   // Wait for each of the child threads to finish.
   for (; i >= 0; i--) {
      pthread_join(threads[i], NULL);
   }

   printf("Client finished.\n");
   return 0;
}

int connectToHost(void) {
   int fd, retval;

   // Allocate a socket for our communication
   fd = socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      printf("ERROR: Could not create socket! Exiting...\n");
      exit(-1);
   }

   // Create the server socket address and zer it.
   struct sockaddr_in server;
   bzero((char*)&server, sizeof(server));

   // We will be using an internet connection (perhaps to localhost though).
   server.sin_family = AF_INET;

   // Copy the host address into the server address structure.
   bcopy((char*)host->h_addr, (char*)&server.sin_addr.s_addr, host->h_length);

   // Format the port into network byte order.
   server.sin_port = htons(portnum);

   // Attempt the actual connection.
   retval = connect(fd, (struct sockaddr*)&server, sizeof(server));
   if (retval < 0 ) {
      printf("ERROR: connect() failed! Exiting...\n");
      exit(-1);
   }

   // If we've made it this far, we will be returning a valid fd entry.
   return fd;
}

void* startPointQueryThread(void* arg) {

   threadargs* args = (threadargs*)arg;
   int i;
   FILE* outfd;
   int fd = connectToHost();
   size_t index;
   double* pt = (double*)malloc(N_DIMENSIONS * sizeof(double));
   char* buf = (char*)malloc(N_DIMENSIONS * DIM_BUFFER_SIZE);
   if (pt == NULL || buf == NULL) {
      printf("ERROR: Could not allocate line buffer! Exiting...\n");
      exit(-1);
   }
   std::fstream in((char*)args->input, fstream::in);

   int log = args->output != NULL;
   TimedQueue* logQueue;
   pthread_t loggerThread;

   if (log) {

      outfd = fopen((char*)args->output, "w+");
      if (outfd == NULL) {
         printf("ERROR: Could not open output file!\n");
         exit(-1);
      }
      Logger logger(fd, fileno(outfd));
      logger.run(&loggerThread);
      logQueue = logger.getQueue();
   }

   while(!in.eof()) {
      in.getline(buf, N_DIMENSIONS * DIM_BUFFER_SIZE - 1);
      if (buf[0] == 't') {
         
         // This is a timestamp, so we may need to wait.
         unsigned long t = atol(&buf[1]);
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

         // This will be an actual point with components separated by commas.
         for (i = 0; i < N_DIMENSIONS; i++) {
            pt[i] = atof(buf);
            buf = strchr(buf, ',') + 1;
         }

         // If we're logging, we need to tell our logger to look for this point.
         logQueue->add(pt);

         // Now, we need to put out the point in a format that the server can
         // interpret properly (including tagging it as a point query).
         if (write(fd, pt, N_DIMENSION * sizeof(double)) !=
                           N_DIMENSION * sizeof(double)) {
            printf("ERROR: Failed to write to server! Exiting!\n");
            exit(-1);
         }
      }
   }  
}

void* startStreamThread(void* arg) {
   return NULL;
}

void* startPolygonQueryThread(void* arg) {
   return NULL;
}


