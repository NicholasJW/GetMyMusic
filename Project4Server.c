
#include "NetworkHeader.h"
char* databaseName = DATABASE_NAME; // Database

struct ThreadArgs {
  int clntSock; // Socket descriptor for client
};

void *handleThread(void *threadArgs){
  pthread_detach(pthread_self());
  int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
  free(threadArgs); // Deallocate memory for argument
  HandleProj4Client(clntSock, databaseName);
  return NULL;
}

int main (int argc, char *argv[])
{
  if (argc != 3) // Test for correct number of arguments
  {
		printf("Error: Usage Project4Server -p <port>\n");
    exit(1);
  } 

  in_port_t servPort = atoi(argv[2]); // Local port

  // Create socket for incoming connections
  int servSock; // Socket descriptor for server
  if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  // Construct local address structure
  struct sockaddr_in servAddr; // Local address
  memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
  servAddr.sin_family = AF_INET; // IPv4 address family
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  servAddr.sin_port = htons(servPort); // Local port 

  // Bind to the local address
  if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    DieWithError("bind() failed");

  // Mark the socket so it will listen for incoming connections
  if (listen(servSock, LISTENQ) < 0)
    DieWithError("listen() failed");

  for (;;)
	{
    struct sockaddr_in clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    // int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    int clntSock= accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0){
      DieWithError("accept() failed");
    }
    struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs));
    pthread_t thread_id;
    if (threadArgs == NULL)
      DieWithError("malloc() failed");
    threadArgs->clntSock = clntSock;
    int returnValue = pthread_create(&thread_id, NULL, handleThread, threadArgs);
    if (returnValue != 0)
      DieWithError("pthread_create() failed");

    // clntSock is connected to a client

  }

  return 0;
}
