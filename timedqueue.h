
#ifndef _TIMED_QUEUE_H_
#define _TIMED_QUEUE_H_

#include <time.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

typedef struct memslab_t {
   memslab_t* next;
   unsigned long entryTime;
   void* data;
} memslab_t;

class TimedQueue {
public:
   TimedQueue(unsigned long elementSize, unsigned long nElements);
   ~TimedQueue();
   unsigned long remove(void* element);
   void add(void* element);
private:
   void* buf;
   memslab_t* freeHead;
   memslab_t* freeTail;
   memslab_t* usedHead;
   memslab_t* usedTail;
   pthread_mutex_t freeLock;
   pthread_mutex_t usedLock;
   unsigned long esize;
};

#endif
