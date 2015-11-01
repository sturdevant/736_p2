
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "timedqueue.h"
#include <unistd.h>

#define N_DIMENSION 2
#define RESPONSE_BUFFER_SIZE 4096
#define QUEUE_SIZE 1024

class Logger {
public:
   Logger(int infd, int outfd);
   ~Logger();
   void run(pthread_t* thread);
   int getInput();
   int getOutput();
   TimedQueue* getQueue();
private:
   int running;
   int input;
   int output;
   TimedQueue* queue;
};

#endif
