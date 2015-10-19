#include <pthread.h>
#include <fstream>
#include <iostream>
#include <stdio>
#include <sys.socket.h>

using namespace std;

#define N_DIMENSIONS 2
#define DIM_BUFFER_SIZE 512

void startPointQueryThread(void* filename);
void startStreamThread(void* filename);
void startPolygonQueryThread(void* filename);

// These globals will be set before any threads and are read-only, making them
// safe for reading in each thread.
struct hostent* host;
int portnum;

int main(int argc, char** argv) {
   if (argc < 5) {
      printf("usage: client <hostname> <port> {[<service type> <filename>], [...]...}\n");
      printf("service types:\n \t\"-point\" query points\n");
      printf("\t\"-stream\" stream, a source of data points\n");
      printf("\t\"-polygon\" query for polygons\n");
      exit(-1);
   }

   host = gethostbyname(argv[1]);
   if (host == NULL) {
      printf("ERROR: Could not resolve host %s! Exiting...\n", argv[1]);
      exit(-1);
   }
   portnum = atoi(argv[2]);

   int i, retval;
   pthread_t* threads = (pthread_t*)malloc((argc / 2 - 1) * sizeof(pthread_t));
   for (i = 0; i < (argc / 2) - 1; i++) {
      int index = 2 * i + 3;
      if (!strcmp(argv[index], "-point") {
         // Create thread to handle point queries
         
         retval = pthread_create(&threads[threadCount], 
                                 NULL, 
                                 &startPointQueryThread,
                                 argv[index + 1]);

      } else if (!strcmp(argv[index], "-stream")) {
         // Create thread to handle streaming data
         
         retval = pthread_create(&threads[threadCount], 
                                 NULL, 
                                 &startStreamThread,
                                 argv[index + 1]);

      } else if (!strcmp(argv[index], "-polygon")) {
         // Create thread to handle polygon requests
         
         retval = pthread_create(&threads[threadCount], 
                                 NULL, 
                                 &startPolygonQueryThread,
                                 argv[index + 1]);

      } else {

         // We don't really need the server to be all that tolerant of input
         // error since it is just a simulation of external calls. If a bad
         // service type is specified, just report it and exit.
         printf("ERROR: Invalid service type %s. Exiting...\n", argv[index]);
         exit(-1);
      }

      // If a thread fails, we'll abort client operation completely.
      if (retval) {
         printf("ERROR: Thread creation failed with %s %s! Exiting...\n",
                argv[index], argv[index + 1]);
         exit(-1);
      }

   }

   // TODO: Timekeeping/cv_signal

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


void startPointQueryThread(void* filename) {
   int i, fd = connectToHost(void);
   size_t index;
   double* pt = (double*)malloc(N_DIMENSIONS * sizeof(double));
   char* buf = (char*)malloc(N_DIMENSIONS * DIM_BUFFER_SIZE);
   if (pt == NULL || buf == NULL) {
      printf("ERROR: Could not allocate line buffer! Exiting...\n");
      exit(-1);
   }
   fstream::fstream in((char*)filename, fstream::in);

   while(!in.eof()) {
      in.getline(buf, BUFFER_SIZE - 1);
      if (buf[0] == 't') {
         
         // This is a timestamp, so we may need to wait.
         int t = atoi(&buf[1]);
         if (t > tick) {
            // This timestamp hasn't come yet, so we need to go to sleep.
            pthread_wait(.....);
         }
      } else {
         
         index = 0;

         // This will be an actual point with components separated by commas.
         for (i = 0; i < N_DIMENSIONS; i++) {
            size_t tempIndex;
            pt[i] = stod((string)buf[(int)index + 1], &tmpIndex);
            index = tmpIndex;
         }

         // Now, we need to put out the point in a format that the server can
         // interpret properly (including tagging it as a point query).
         write(fd.....);
      }
   }  
}

void startStreamThread(void* filename);
void startPolygonQueryThread(void* filename);


