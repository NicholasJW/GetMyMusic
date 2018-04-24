
#include "NetworkHeader.h"
#include "WhoHeader.h"

/* function declarations */

// given a packet, looks at the length field (4-5th bytes) and returns
// its corresponding decimal value as unsigned long
unsigned long retrieveLength(char* packet);

// Constructs and sends LEAVE message to server.
void sendLEAVE(int sock);

// Prints every song names in given packet.
// numSongs represents number of songs in packet
// First two bytes: length field, then song name, SHA, song name, SHA,...
void printDIFF(char* packet, unsigned long numSongs);

// prints every song and SHA combination from listResponse.
// numEntries represents number of song and SHA combinations in listResponse.
void printLIST(char* listResponse, unsigned long numEntries);

// Constructs and sends LIST message to server.
void sendLIST(int sock);

// Constructs and sends PUSH message to server
void sendPUSH(int sock, unsigned long messageLen, char* songName, char* SHA, char* songFile, int songSize);

// Constructs and sends PULL message to server
void sendPULL(int sock, char* SHA);

// Receives and sets response packet to response.
// Returns length field of response pakcet as unsigned long.
unsigned long receiveResponse(int sock, char* response);

/*
Called when the user enters the 'DIFF' command on the client side
LIST message format 1st 4 chars = message type 'list' next 2 = length, then song:sha.....
*/
void handleDiff(char* receivedList, unsigned long length, char* songs_Not_On_Client, char* songs_Not_On_Server);


int main (int argc, char *argv[])
{

  /* There are certainly slicker ways of doing this,
   * but I figured there's no need to make the code
   * any more confusing than it has to be at this point.
   */

  // Argument parsing variables
  char *serverHost = SERVER_HOST;
  char *serverPort = SERVER_PORT; 
  char *servPortString;
  char c;
  int i;

  if ((argc < 1) || (argc > 3)) {
    printf("Error: Usage Project0Client [-s <server IP/Name>[:<port>]]\n");
    exit(1);
  }

  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      c = argv[i][1];

      /* found an option, so look at next
       * argument to get the value of 
       * the option */
      switch (c) {

      case 's':
	serverHost = strtok(argv[i+1],":");
	if ((servPortString = strtok(NULL, ":")) != NULL) {
	  serverPort = servPortString;
	}
	break;
      default:
	break;
      }
    }
  }

	// create a connected TCP socket
	int sock = SetupTCPClientSocket(serverHost, serverPort);
	if (sock < 0)
		DieWithError("SetupTCPClientSocket() failed");

	// open database file
	open_database("database.dat");

	// ask user for command (list, diff, sync, leave)
	printf("Enter Command in Small Case: ");
	char* command = (char*) malloc(5); 
	scanf("%s", command);
	
	while (strcmp(command, "leave") != 0)
	{
		if (strcmp(command, "list") == 0)
		{

			// send LIST message to server
			sendLIST(sock);

			// receive listResponse from server
			char listResponse[BUFFSIZE];
			unsigned long length_Message = receiveResponse(sock, listResponse);
			
			// print every song from listResponse
			printLIST(listResponse, length_Message);

			// read another command from user
			printf("Enter Another Command in Small Case: ");
			scanf("%s", command);
		}

		else if (strcmp(command, "pull") == 0)
		{
			// construct PULL message
			char pullMessage[BUFFSIZE];
			strcpy(pullMessage, PULLType);

			// message length is SHA_LENGTH
			pullMessage[5] = (uint16_t)SHA_LENGTH;
			pullMessage[4] = (uint16_t)SHA_LENGTH >> 8;


						unsigned long lengthMessage = retrieveLength(pullMessage);
						printf("lengthMessage: %lu\n", lengthMessage); // DEBUGGING

			// DEBUGGING
			char tmpSHA[128] = "00000000011111111222222222222222222221222111111111111122222222222222222222N12221111111111111222222222222222222222222222222222228";
			int k;
			for (k = 0; k < 128; k++)
			{
				pullMessage[6+k] = tmpSHA[k];
			}
			// DEBUGGING print pullmessage that is about to be sent to server
			int j;
			for (j = 0; j < 134; j++)
			{
				printf("%c", pullMessage[j]);
			}
			printf("\n");

			// send the PULL message to the server
			ssize_t numBytes = send(sock, pullMessage, 4+2+SHA_LENGTH, 0);
			//printf("numBytes sent for PULL: %zu\n", numBytes); // debugging
			if (numBytes < 0)
				DieWithError("send() failed");
			else if (numBytes != 4+2+SHA_LENGTH)
				DieWithError("send() failed: sent unexpected number of bytes");

			// receive pullResponse message from server
			char pullResponse[BUFFSIZE]; // create pullResponse
			receiveResponse(sock, pullResponse);

			// retrieve song name
			char songName[MAX_SONGNAME_LENGTH+1];
			strncpy(songName, pullResponse+4+2, MAX_SONGNAME_LENGTH);

			// retrieve song file
			char song[MAX_SONG_LENGTH+1];
			strncpy(song, pullResponse+4+2+MAX_SONGNAME_LENGTH, MAX_SONG_LENGTH);

			// print song name and song file DEBUGGING
			printf("%s \t %s\n", songName, song);

			printf("Enter Another Command in Small Case: ");
			scanf("%s", command); // wait for another command
		}
		
		// DEBUGGING
		else if (strcmp(command, "push") == 0)
		{
			// construct PUSH message
			char pushMessage[BUFFSIZE];
			strcpy(pushMessage, PUSHType);

			// append message length
			unsigned long messageLen = MAX_SONGNAME_LENGTH + SHA_LENGTH + 15;
			printf("ORIGINAL messageLen: %lu\n", messageLen);
			pushMessage[5] = (uint16_t)messageLen;
			pushMessage[4] = (uint16_t)messageLen >> 8;
			
						unsigned long lengthMessage = retrieveLength(pushMessage);
						printf("lengthMessage: %lu\n", lengthMessage); // DEBUGGING

			char songName[255] = "Name2";
			// append null characters for the rest of songName
			int r;
			for (r = MAX_SONGNAME_LENGTH; r >= 5; r--)
			{
				songName[r] = '\0';
			}

			int i;
			for (i = 0; i < 255; i++)
			{
				pushMessage[6+i] = songName[i];
			}

			char sha[128] = "00000000011111111222222222222222222221222111111111111122222222222222222222N12221111111111111222222222222222222222222222222222228";
			for (i = 0; i < 128; i++)
			{
				pushMessage[6+255+i] = sha[i];
			}

			char file[15] = "0124810354 6242";
			for (i = 0; i < 128; i++)
			{
				pushMessage[6+255+128+i] = file[i];
			}

			// print pushMessage
			for (i = 0; i < 6+255+128+15; i++)
			{
				printf("%c", pushMessage[i]);
			}
			printf("\n");

			// send pushMessage to server
			ssize_t numBytes = send(sock, pushMessage, 4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH+15, 0); // 15 MUST BE REPLACED BY FILE LENGTH LATER
			if (numBytes < 0)
				DieWithError("send() failed");
			else if (numBytes != 4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH+15) // 15 MUST BE REPLACED BY FILE LENGTH LATER
				DieWithError("send() failed: sent unexpected number of bytes");
			
			printf("Enter Another Command in Small Case: ");
			scanf("%s", command);
		}

		else if (strcmp(command, "diff") == 0)
		{
			printf("ENTERED DIFF!!!\n");
			// send LIST message to server
			sendLIST(sock);

			// receive listResponse from server
			char listResponse[BUFFSIZE];
			unsigned long length_Message = receiveResponse(sock, listResponse);

			// calculate different song files between client and server
			char* clientSongs = compareSongsToClient(listResponse+6, length_Message); // songs that are in client but not in server
			char* serverSongs = compareSongsToServer(listResponse+6, length_Message); // songs that are in server but not in client

			// retrieve lengths of clientSongs and serverSongs
			unsigned long lengthClient = getLength(clientSongs);
			unsigned long lengthServer = getLength(serverSongs);

			// print songs in client but not in server
			printf("Songs in client but not in server: \n");
			printDIFF(clientSongs, lengthClient);

			// print songs in server but not in client
			printf("Songs in server but not in client: \n");
			printDIFF(serverSongs, lengthServer);

			// wait for another command
			printf("Enter Another Command in Small Case: ");
			scanf("%s", command);
		}

		else if (strcmp(command, "sync") == 0)
		{
			// send LIST message to server
			sendLIST(sock);

			// receive listResponse from server
			char listResponse[BUFFSIZE];
			unsigned long length_Message = receiveResponse(sock, listResponse);

			// calculate different song files between client and server
			char* clientSongs = compareSongsToClient(listResponse+6, length_Message); // songs that are in client but not in server
			char* serverSongs = compareSongsToServer(listResponse+6, length_Message); // songs that are in server but not in client


			// retrieve lengths of clientSongs and serverSongs
			unsigned long lengthClient = getLength(clientSongs);
			unsigned long lengthServer = getLength(serverSongs);

/*// DEBUGGING
			printf("clientSongs: ");
			for (i = 0; i < 2+lengthClient*(MAX_SONGNAME_LENGTH+SHA_LENGTH); i++)
			{
				printf("%c", clientSongs[i]);
			}
			printf("\n");

			printf("serverSongs: ");
			for (i = 0; i < 2+lengthServer*(MAX_SONGNAME_LENGTH+SHA_LENGTH); i++)
			{
				printf("%c", serverSongs[i]);
			}
			printf("\n");

			printf("lengthMessage: %lu\n", lengthClient); // DEBUGGING
			printf("lengthMessage: %lu\n", lengthServer); // DEBUGGING
*/

			// push all the songs from clientSongs to server 
			int i;
			for (i = 0; i < lengthClient; i++)
			{
				// retrieve song name from clientSongs
				char songName[MAX_SONGNAME_LENGTH+1];
				strncpy(songName, clientSongs + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH), MAX_SONGNAME_LENGTH);
				songName[MAX_SONGNAME_LENGTH] = '\0';
				printf("pushing songName: %s\n", songName);

				// retrieve SHA from clientSongs
				char SHA[SHA_LENGTH+1];
				strncpy(SHA, clientSongs + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH) + MAX_SONGNAME_LENGTH, SHA_LENGTH);
				SHA[SHA_LENGTH] = '\0';
				
				// get the song file from database using name and SHA
				char songFile[MAX_SONG_LENGTH];
				int songSize;
				getSong(songName, songFile, &songSize);
				songSize = songSize - 2; // has to do with how file works(?)
				//printf("pushing SONG SIZE: %i\n", songSize); // DEBUGGING
				songFile[songSize] = '\0'; // append null-terminator
				printf("pushing song file: %s\n", songFile); // DEBUGGING

				// construct and send PUSH message to server
				int length_field = MAX_SONGNAME_LENGTH+SHA_LENGTH+songSize; // calculate length field in push message
				sendPUSH(sock, length_field, songName, SHA, songFile, songSize);

				// receive pushResponse from server
				char pushResponse[BUFFSIZE]; // create pushResponse
				receiveResponse(sock, pushResponse);
			}

			// pull all the songs from serverSongs to client
			for (i = 0; i < lengthServer; i++)
			{
				// retrieve song name from serverSongs
				char songName[MAX_SONGNAME_LENGTH+1];
				strncpy(songName, serverSongs + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH), MAX_SONGNAME_LENGTH);
				songName[MAX_SONGNAME_LENGTH] = '\0';
				printf("pulling songName: %s\n", songName); // DEBUGGING

				// retrieve SHA from serverSongs
				char SHA[SHA_LENGTH+1];
				strncpy(SHA, serverSongs + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH) + MAX_SONGNAME_LENGTH, SHA_LENGTH);
				SHA[SHA_LENGTH] = '\0';

				// send PULL message
				sendPULL(sock, SHA);

				printf("SENT PULL MESSAGE!!!\n");

				// receive pullResponse message from server
				char pullResponse[BUFFSIZE]; // create pullResponse
				receiveResponse(sock, pullResponse);

				// retrieve song file
				char song[MAX_SONG_LENGTH+1];
				strncpy(song, pullResponse+4+2, MAX_SONG_LENGTH);
				song[MAX_SONG_LENGTH] = '\0';

				// song file length is precisely length field in pullResponse
				int songLength = retrieveLength(pullResponse);

				printf("pulled song length: %i\n", songLength);

				// store song in database
				storeSong(songName, song, songLength); // stores song file in this directory
				addSong(songName, SHA); // stores song in txt file that keeps track of list of song and SHA combinations
			}

			printf("Enter Another Command in Small Case: ");
			scanf("%s", command); // wait for another command

		}

	}

	// send LEAVE message to server
	sendLEAVE(sock);

	return 0;
}


// Constructs and sends LEAVE message to server.
void sendLEAVE(int sock)
{
	// Construct LEAVE message
	char leaveMessage[BUFFSIZE];
	strcpy(leaveMessage, LEAVEType);
	// length field is zero
	leaveMessage[4] = 0x0;
	leaveMessage[5] = 0x0;

	// send LEAVE message to server
	ssize_t numBytesSent = send(sock, leaveMessage, 4+2, 0);
	if (numBytesSent < 0)
	{
		DieWithError("send() failed");
	}
}

// Prints every song names in given packet.
// numSongs represents number of songs in packet
// First two bytes: length field, then song name, SHA, song name, SHA,...
void printDIFF(char* packet, unsigned long numSongs)
{
		int i;
		for (i = 0; i < numSongs; i++)
		{
			// retrieve song name from packet
			char songName[MAX_SONGNAME_LENGTH+1];
			strncpy(songName, packet + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH), MAX_SONGNAME_LENGTH);
			songName[MAX_SONGNAME_LENGTH] = '\0';

			printf("%s\n", songName);
		}
		printf("\n");
} 

// Constructs and sends LIST message to server.
void sendLIST(int sock)
{
	// construct LIST message
	char listMessage[BUFFSIZE];
	strcpy(listMessage, LISTType);
	// length field is zero
	listMessage[4] = 0x0;
	listMessage[5] = 0x0;

	printf("LIST TYPE??: ");
	int i;
	for (i = 0; i < 4; i++)
	{
		printf("%c", listMessage[i]);
	}
	printf("\n");
	
	// send LIST message to server
	ssize_t numBytesSent = send(sock, listMessage, 4+2, 0);
	if (numBytesSent < 0)
	{
		DieWithError("send() failed");
	}
}


// prints every song and SHA combination from listResponse.
// numEntries represents number of song and SHA combinations in listResponse.
void printLIST(char* listResponse, unsigned long numEntries)
{
	// print the names of the songs in the server to stdout
	printf("Song name\n");
	int i;
	for (i = 0; i < numEntries/(MAX_SONGNAME_LENGTH+SHA_LENGTH); i++)
	{
		// retrieve song name
		char currentSongName[MAX_SONGNAME_LENGTH+1];
		strncpy(currentSongName, listResponse+4+2+i*(MAX_SONGNAME_LENGTH+SHA_LENGTH), MAX_SONGNAME_LENGTH);
		currentSongName[MAX_SONGNAME_LENGTH] = '\0';
	
		// retrieve SHA DEBUGGING
		char currentSHA[SHA_LENGTH+1];
		strncpy(currentSHA, listResponse+4+2+i*(MAX_SONGNAME_LENGTH+SHA_LENGTH)+MAX_SONGNAME_LENGTH, SHA_LENGTH);
		currentSHA[SHA_LENGTH] = '\0';

		// print song name 
		printf("%s\n", currentSongName);
	}
}


// Constructs and sends PUSH message to server
void sendPUSH(int sock, unsigned long messageLen, char* songName, char* SHA, char* songFile, int songSize)
{
	// construct PUSH message
	char pushMessage[BUFFSIZE];
	strcpy(pushMessage, PUSHType);

	// append message length
	printf("ORIGINAL messageLen: %lu\n", messageLen); // DEBUGGING
	pushMessage[5] = (uint16_t)messageLen;
	pushMessage[4] = (uint16_t)messageLen >> 8;
			
						unsigned long lengthMessage = retrieveLength(pushMessage);
						printf("pushed lengthMessage: %lu\n", lengthMessage); // DEBUGGING

	// append songName
	int i;
	for (i = 0; i < MAX_SONGNAME_LENGTH; i++)
	{
		pushMessage[4+2+i] = songName[i];
	}

	// append SHA
	for (i = 0; i < SHA_LENGTH; i++)
	{
		pushMessage[4+2+MAX_SONGNAME_LENGTH+i] = SHA[i];
	}

	// append songFile
	for (i = 0; i < songSize; i++)
	{
		pushMessage[4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH+i] = songFile[i];
	}

	printf("push Message SENT: ");
	for (i = 0; i < 4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH+songSize; i++)
	{
		printf("%c", pushMessage[i]);
	}
	printf("\n");

	// send pushMessage to server
	ssize_t numBytes = send(sock, pushMessage, 4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH+songSize, 0);
	if (numBytes < 0)
		DieWithError("send() failed");
	else if (numBytes != 4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH+songSize)
		DieWithError("send() failed: sent unexpected number of bytes");
}

// Constructs and sends PULL message to server
void sendPULL(int sock, char* SHA)
{
	// construct PULL message
	char pullMessage[BUFFSIZE];
	strcpy(pullMessage, PULLType);

	// message length is SHA_LENGTH
	pullMessage[5] = (uint16_t)SHA_LENGTH;
	pullMessage[4] = (uint16_t)SHA_LENGTH >> 8;

	printf("length of pull message sent: %lu\n", retrieveLength(pullMessage));

	// append SHA to pullMessage
	int k;
	for (k = 0; k < SHA_LENGTH; k++)
	{
		pullMessage[4+2+k] = SHA[k];
	}

	printf("pull Message SENT: ");
	for (k = 0; k < 4+2+SHA_LENGTH; k++)
	{
		printf("%c", pullMessage[k]);
	}
	printf("\n");

	// send the PULL message to the server
	ssize_t numBytes = send(sock, pullMessage, 4+2+SHA_LENGTH, 0);
	//printf("numBytes sent for PULL: %zu\n", numBytes); // debugging
	if (numBytes < 0)
		DieWithError("send() failed");
	else if (numBytes != 4+2+SHA_LENGTH)
		DieWithError("send() failed: sent unexpected number of bytes");
}


/*
Called when the user enters the 'DIFF' command on the client side
LIST message format 1st 4 chars = message type 'list' next 2 = length, then song:sha.....
*/
void handleDiff(char* receivedList, unsigned long length, char* songs_Not_On_Client, char* songs_Not_On_Server)
{

	songs_Not_On_Client = compareSongsToClient(receivedList, length);
	//printf("SONGS NOT ON CLIENT:\n%s \n", songs_Not_On_Client);

	songs_Not_On_Server = compareSongsToServer(receivedList, length);
	//printf("SONGS NOT ON SERVER:\n%s \n", songs_Not_On_Server);

}

