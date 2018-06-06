
#include "NetworkHeader.h"
#include "WhoHeader.h"

// receives and sets response packet to response
// unsigned long receiveResponse(int sock, char* response);

void HandleProj4Client(int cliSock, char *databaseName)
{
	char* musicDir = "serverSong/";
	// open database file
	open_database(databaseName, musicDir);

	for (;;) // breaks when leave message is received
	{
		// receive message from client
		char rcvMessage[BUFFSIZE];
		unsigned long length_Message = receiveResponse(cliSock, rcvMessage);
	
		// Check the message type (first 4 bytes in the message)
		char typeField[5];
		int i;
		for (i = 0; i < 4; i++)
		{
			typeField[i] = rcvMessage[i];
		}
		typeField[4] = '\0';

		printf("type of message received: %s\n", typeField); // debugging


		// Received LIST message
		// LIST message contains nothing but the message type and length field (which is zero).
		if (strcmp(typeField, LISTType) == 0)
		{
			// retrieve all the songs from the database
			int numEntries; // specifies number of songs
			char** songs = lookup_songs(&numEntries);

			char listResponse[BUFFSIZE];
			strcpy(listResponse, LISTType);

			// find song names and SHA's from database and store in listResponse packet
			int i;
			for (i = 0; i < numEntries; i++)
			{
				char** oneSong = songs+i;
			
				// find the first occurence of ':'
				int k; int firstIndex;
				for (k = 0; k < MAX_SONGNAME_LENGTH+1; k++) // at most MAX_SONGNAME_LENGTH characters before first ':'
				{
					if ((*oneSong)[k] == ':')
					{
						firstIndex = k;
						break;
					}
				}
			
				// retrieve song name
				char songName[MAX_SONGNAME_LENGTH+1];
				strncpy(songName, (*oneSong), firstIndex);
				// append null characters for the rest of songName
				int r;
				for (r = MAX_SONGNAME_LENGTH; r >= firstIndex; r--)
				{
					songName[r] = '\0';
				}
				//printf("songName: %s\n", songName); // debugging

				// retrieve SHA 
				char sha[SHA_LENGTH+1];
				strcpy(sha, (*oneSong)+firstIndex+1);
				//printf("SHA: %s\n", sha); // debugging

				// store song name in listResponse packet
				int j;
				for (j = 0; j < MAX_SONGNAME_LENGTH; j++) // 30 bytes used for song name
				{
					listResponse[4 + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH) + j] = songName[j]; // 4 bytes for "LIST", 2 bytes for length field
					//printf("character appended: %c\n", songName[j]); // debugging
				}

				// store SHA listResponse packet
				int y;
				for (y = 0; y < SHA_LENGTH; y++) // 128 bytes used for SHA
				{
					listResponse[4 + 2 + i*(MAX_SONGNAME_LENGTH+SHA_LENGTH) + MAX_SONGNAME_LENGTH + y] = sha[y]; // 4 bytes for "LIST", 2 bytes for length field
					//printf("character appended: %c\n", sha[y]); // debugging
				}


			}

			// fill length field in 4th-5th bits of listResponse packet
			listResponse[5] = (uint16_t)numEntries*(MAX_SONGNAME_LENGTH+SHA_LENGTH);
			listResponse[4] = (uint16_t)numEntries*(MAX_SONGNAME_LENGTH+SHA_LENGTH) >> 8;

			/*//print listResponse DEBUGGING
			int j;
			for (j = 0; j < 4 + 2 + numEntries*(MAX_SONGNAME_LENGTH+SHA_LENGTH); j++)
			{
				printf("%c", listResponse[j]);
			}
			printf("\n");*/

			// send listResponse packet
			int length_response = 4 + 2 + numEntries*(MAX_SONGNAME_LENGTH+SHA_LENGTH); // length of listResponse packet
			ssize_t numBytesSent = send(cliSock, listResponse, length_response, 0);
			if (numBytesSent < 0)
			{
				DieWithError("send() failed");
			}
		}


		// Received PULL message
		// PULL message contains type field, length field (2 bytes), and SHA.
		else if (strcmp(typeField, PULLType) == 0)
		{
			/*
			// DEBUGGING
			printf("rcvMessage: ");
			int i;
			for (i = 0; i < totalBytesRcvd; i++)
			{
				printf("%c", rcvMessage[i]);
			}
			printf("\n");*/

			// retrieve SHA from PULL message
			char SHA[SHA_LENGTH+1];
			strcpy(SHA, rcvMessage+6);
			// printf("SHA RECEIVED: %s\n", SHA); // debugging

			// get the song name corresponding to SHA
			char songName[MAX_SONGNAME_LENGTH+1];
			strcpy(songName, getSongName(SHA));
			printf("songName: %s\n", songName); // DEBUGGING 
		
			// get the actual song file from current directory
			char song[MAX_SONG_LENGTH];
			int songSize;
			getSong(songName, song, &songSize);
			songSize = songSize - 2; // has to do with how file works(?)
			// printf("SONG SIZE: %i\n", songSize); // DEBUGGING
			song[songSize] = '\0'; // append null-terminator
			// printf("SONG TO SEND: %s\n", song); // DEBUGGING

			// create response message
			char pullResponse[4 + 2 + songSize]; // extra 4 for type field, 2 for length field
			strcpy(pullResponse, PULLType);
			pullResponse[5] = (uint16_t)songSize;
			pullResponse[4] = (uint16_t)songSize >> 8;
			strncpy(pullResponse+4+2, song, songSize);
		
			// send response to client
			ssize_t numBytesSent = send(cliSock, pullResponse, 4+2+songSize, 0);
			if (numBytesSent < 0)
			{
				DieWithError("send() failed");
			}
		}

		// Received PUSH message
		// PUSH message contains type field, length field (2 bytes), song name, and song file.
		else if (strcmp(typeField, PUSHType) == 0)
		{
			// retrieve song name
			char songName[MAX_SONGNAME_LENGTH+1];
			strncpy(songName, rcvMessage+4+2, MAX_SONGNAME_LENGTH);
			songName[MAX_SONGNAME_LENGTH] = '\0';

			// retrieve SHA
			char SHA[SHA_LENGTH+1];
			strncpy(SHA, rcvMessage+4+2+MAX_SONGNAME_LENGTH, SHA_LENGTH);
			SHA[SHA_LENGTH] = '\0';

			// retrieve song file
			int songLength = length_Message - MAX_SONGNAME_LENGTH - SHA_LENGTH; // calculate length of song file
			char song[MAX_SONG_LENGTH+1];
			strncpy(song, rcvMessage+4+2+MAX_SONGNAME_LENGTH+SHA_LENGTH, songLength);
			song[songLength] = '\0';

/* //DEBUGGING
			int i;
			printf("songName: ");
			for (i=0; i < 255; i++)
				printf("%c", songName[i]);
			printf("\n");

			int j;
			printf("SHA: ");
			for (j=0; j < 128; j++)
				printf("%c", SHA[j]);
			printf("\n"); */

			// store song in database
			storeSong(songName, song, songLength); // stores song file in this directory
			addSong(songName, SHA); // stores song in txt file that keeps track of list of song and SHA combinations

			// create response message
			char pushResponse[4 + 2]; // extra 4 for type field, 2 for length field
			strcpy(pushResponse, PUSHType);
			pushResponse[5] = 0x0;
			pushResponse[4] = 0x0;
		
			// send response to client
			ssize_t numBytesSent = send(cliSock, pushResponse, 4+2, 0);
			if (numBytesSent < 0)
			{
				DieWithError("send() failed");
			}
		}

		else if (strcmp(typeField, LEAVEType) == 0)
		{
			close_database(); // Close database
			close(cliSock); // Close client socket
			return;
		}

		else
		{
			// do nothing
		}
	}
}
