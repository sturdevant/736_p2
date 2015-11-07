#include <stdio.h>
#include <sys/socket.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define N_DIMENSIONS 2

int main(int argc, char** argv) {
   int listenfd, newfd, portnum;
   char buf[256];
   struct sockaddr_in server, client;

   if (argc != 2) {
      std::cout << "Usage: server <port number>" << std::endl;
      exit(0);
   }

   portnum = atoi(argv[1]);

   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   if (listenfd < 0) {
      std::cout << "Could not open socket!" << std::endl;
      exit(0);
   }

   bzero((char*)&server, sizeof(struct sockaddr_in));
   bzero((char*)&client, sizeof(struct sockaddr_in));

   server.sin_family = AF_INET;
   server.sin_port = htons(portnum);
   server.sin_addr.s_addr = INADDR_ANY;

   socklen_t sockLen = sizeof(sockaddr);

   if (bind(listenfd, (struct sockaddr*)&server, sockLen) < 0) {
      std::cout << "Could not bind to socket!" << std::endl;
      exit(0);
   }
   listen(listenfd, 5);

   newfd = accept(listenfd, (struct sockaddr*)&client, &sockLen);
   if (newfd < 0) {
      std::cout << "ERROR: newfd does not specify a valid entry!" << std::endl;
      exit(0);
   }

   bzero(buf, 256);
   size_t len = read(newfd, buf, 255);
   if (len < 0) {
      std::cout << "ERROR: Could not read from socket!" << std::endl;
      exit(0);
   }

   char type = *buf;
   double* pt = (double*)&buf[1];
   printf("%02X\n", type);
   std::cout << "len = " << len << " Type: " << type
             << " X = " << pt[0] 
             << " Y = " << pt[1] 
             << std::endl;

   return 0;  

}
