#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#define BUFFER_SIZE 4096
#define TIME_PER_FILE 300

int main(int argc, char** argv) {

   time_t startTime, curTime, nextTime;
   size_t readSize, writeSize;
   char* buf = (char*)malloc(BUFFER_SIZE);
   unsigned int curFile = 0;
   char* filename = (char*)malloc(256);
   filename[255] = 0;
   char* baseFilename = "data/twitterdata";
   char* fileExtension = ".json";
   FILE* out;

   sprintf(filename, "%s%d%s", baseFilename, curFile, fileExtension);
   out = fopen(filename, "w");
   if (out == NULL) {
      printf("ERROR! Could not open file %s\n", filename);
      exit(1);
   }

   startTime = time(NULL);
   nextTime = startTime + TIME_PER_FILE;

   while (1) {
      readSize = read(STDIN_FILENO, buf, BUFFER_SIZE - 1);
      if (readSize <= 0) {
         fclose(out);
         printf("ERROR! Could not read from file!\n");
         exit(1);
      }
      //printf("Read %s\n", buf);

      curTime = time(NULL);
      if (curTime >= nextTime) {

         fclose(out);
         curFile++;
         sprintf(filename, "%s%d%s", baseFilename, curFile, fileExtension);
         out = fopen(filename, "w");
         if (out == NULL) {
            printf("ERROR! Could not open file %s\n", filename);
            exit(1);
         }

         nextTime += TIME_PER_FILE;
      }

      //writeSize = 0;
      //while (readSize > 0) {
         size_t retval = fwrite(buf, readSize, 1, out);
         if (retval != 1) {
            printf("ERROR! Could not write to file!\n");
            exit(1);
         }
         //readSize -= retval;
      //}
   }
}
