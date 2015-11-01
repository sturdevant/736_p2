
#include "logger.h"

void* loggerThreadStart(void* args);

Logger::Logger(int infd, int outfd) {
   input = infd;
   output = outfd;
   running = 0;
   queue = new TimedQueue(N_DIMENSION * sizeof(double), QUEUE_SIZE);
}

Logger::~Logger() {
   delete queue;
}

void Logger::run(pthread_t* thread) {
   if (running) {
      throw "ERROR: Logger already running!\n";
   }

   running = 1;
   if (pthread_create(thread, NULL, &loggerThreadStart, this)) {
      throw "ERROR: Could not start logging thread!\n";
   }
}

int Logger::getInput() {
   return input;
}

int Logger::getOutput() {
   return output;
}

void* loggerThreadStart(void* args) {

   Logger* log = (Logger*)args;
   TimedQueue* queue = log->getQueue();
   int output = log->getOutput();
   int input = log->getInput();

   int lsize = sizeof(long);
   int dsize = sizeof(double);

   // Now we begin the actual logging code.
   void* hbuf = malloc(lsize);
   void* inbuf = malloc(RESPONSE_BUFFER_SIZE);
   if (hbuf == NULL || inbuf == NULL) {
      throw "ERROR: Could not allocate logging buffer!\n";
   }

   int headerBytes = 0;

   // Now we descend into the loop of dequeueing points and recording them.
   while (1) {
      headerBytes += read(input, 
                         ((char*)hbuf) + headerBytes,
                         lsize - headerBytes);

      if (headerBytes == lsize) {
         headerBytes = 0;
         unsigned long responseLen = *((unsigned long*)hbuf);
         int responseBytes = 0;

         while (responseBytes < N_DIMENSION * dsize) {
            responseBytes += read(input, 
                                 ((char*)inbuf) + responseBytes,
                                 N_DIMENSION * dsize - responseBytes);
         }

         double responseTime = queue->remove((double*)inbuf);
         long outputLen = responseLen + 1;

         // Now, print out the response time and the point info.
         if (write(output, &outputLen, lsize) != lsize) {
            throw "ERROR: Failed to write to file!\n";
         }

         if (write(output, &responseTime, dsize)) {
            throw "ERROR: Failed to write to file!\n";
         }

         if (write(output, inbuf, N_DIMENSION * dsize) != N_DIMENSION * dsize) {
            throw "ERROR: Failed to write point to file!\n";
         }

         // We've read the header and the point, now we need to just copy the
         // response.
         while (responseBytes < responseLen) {
            int readLen = RESPONSE_BUFFER_SIZE;
            if (responseLen - responseBytes < readLen) {
               readLen = responseLen - responseBytes;
            }
            int nBytes = read(input, inbuf, readLen);
            if (write(output, inbuf, nBytes) != nBytes) {
               throw "ERROR: Failed to write out response data!\n";
            }
            responseBytes += nBytes;
         }
      }
   }        
}

TimedQueue* Logger::getQueue() {
   return queue;
}


