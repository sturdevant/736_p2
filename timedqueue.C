
#include "timedqueue.h"

TimedQueue::TimedQueue(unsigned long elementSize, unsigned long nElements) {
   esize = elementSize;

   usedHead = NULL;
   usedTail = NULL;

   if (nElements == 0) {
      freeHead = NULL;
      freeTail = NULL;
      return;
   }

   unsigned long slabSize = esize + sizeof(memslab_t*) + sizeof(unsigned long);
   unsigned long bufSize = nElements * slabSize;

   // We need to malloc space for the specified number of elements and the
   // appropriate bookeeping per element.
   buf = malloc(bufSize);

   if (buf == NULL) {
      throw "ERROR: Could not allocate TimedQueue buffer!\n";
   }

   // Now, we need to initialize the locks and traverse the buffer to create
   // a proper free and used list.
   freeHead = (memslab_t*)buf;
   freeTail = (memslab_t*)(((char*)buf) + bufSize - slabSize);
   freeTail->next = NULL;

   memslab_t* cur = freeHead;
   while (cur < freeTail) {
      cur->next = (memslab_t*)(((char*)cur) + slabSize);
      cur = cur->next;
   }

   count = 0;

   // Time to make locks
   pthread_mutex_init(&freeLock, NULL);
   pthread_mutex_init(&usedLock, NULL);

}

TimedQueue::~TimedQueue() {
   free(buf);
}

unsigned long TimedQueue::remove(void* element) {
   
   unsigned long slab;
   int psize = sizeof(void*);
   int lsize = sizeof(unsigned long);

   // Must set t here
   struct timespec ts;
   int success = clock_gettime(CLOCK_REALTIME, &ts);
   unsigned long t = ts.tv_sec * 1000000000 + ts.tv_nsec;

   pthread_mutex_lock(&usedLock);
   if (usedHead == NULL) {
      std::cout << "ERROR: Attempt to remove from empty queue!\n";
      throw "ERROR: Queue empty!\n";
   }
   void* tail = usedTail;
   pthread_mutex_unlock(&usedLock);

   memslab_t* prev = NULL;
   memslab_t* cur = usedHead;
   int found = 0;
   while (cur != tail && !found) {
      found = !memcmp(element, &(cur->data), esize);
      if (!found) {
         prev = cur;
         cur = cur->next;
      }
   }

   if (!found && memcmp(element, &(cur->data), esize)) {
      std::cout << "ERROR: Could not find element in queue!\n";
      throw "ERROR: Queue did not contain point!\n";
   }
   
   unsigned long t2 = cur->entryTime;

   // First, delete the item from the list of used items.
   pthread_mutex_lock(&usedLock);
   count--;
   if (cur == usedHead) {
      usedHead = usedHead->next;
      if (usedHead == NULL) {
         usedTail = NULL;
      }
   } else if (prev != NULL) {
      prev->next = cur->next;
      if (cur == usedTail) {
         usedTail = prev;
      }
   } else {
      usedTail = NULL;
      usedHead = NULL;
   }
   pthread_mutex_unlock(&usedLock);

   // Next, add the slab to the list of free slabs.
   pthread_mutex_lock(&freeLock);
   if (freeTail != NULL) {
      freeTail->next = cur;
      freeTail = cur;
   } else {
      freeTail = cur;
      freeTail->next = NULL;
   }
   pthread_mutex_unlock(&freeLock);

   return t - t2;
}

void TimedQueue::add(void* element) {

   memslab_t* slab;
   int psize = sizeof(void*);
   int lsize = sizeof(unsigned long);

   // Must set t here
   struct timespec ts;
   int success = clock_gettime(CLOCK_REALTIME, &ts);
   unsigned long t = ts.tv_sec * 1000000000 + ts.tv_nsec;

   pthread_mutex_lock(&freeLock);
   if (freeHead == NULL) {
      std::cout << "ERROR: Queue full. Could not add element to queue!\n";
      throw "ERROR: Queue full!\n";
   } else {
      slab = freeHead;
      if (freeTail == freeHead) {
         freeHead = NULL;
         freeTail = NULL;
      } else {
         freeHead = freeHead->next;
      }
   }
   pthread_mutex_unlock(&freeLock);
   
   slab->entryTime = t;
   bcopy(element, &(slab->data), esize);

   pthread_mutex_lock(&usedLock);
   count++;
   //std::cout << "Queue contains " << count << " elements (h = " << usedHead << " t = " << usedTail << "!\n";
   slab->next = NULL;
   if (usedTail == NULL) {
      usedHead = slab;
   } else {
      usedTail->next = slab;
   }
   usedTail = slab;
   pthread_mutex_unlock(&usedLock);
}
