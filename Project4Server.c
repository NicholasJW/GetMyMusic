
#include "NetworkHeader.h"

int main (int argc, char *argv[])
{
  if (argc != 3) // Test for correct number of arguments
  {
		printf("Error: Usage Project4Server -p <port>\n");
    exit(1);
  } 

  char* databaseName = DATABASE_NAME; // Database
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
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
      DieWithError("accept() failed");

    // clntSock is connected to a client

    HandleProj4Client(clntSock, databaseName);

  }

  return 0;
}
