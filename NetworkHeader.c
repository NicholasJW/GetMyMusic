#include "NetworkHeader.h"

/*
 * Print the error message to stderr and exit the program.
 */
void DieWithError(const char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

unsigned int getLength(char* field)
{
	printf("INT VERSION OF GETLENGTH GOT CALLED!!!!\n");
	char firstBin[17]; char secondBin[9];
	byte_to_binary(field[0], firstBin);
	byte_to_binary(field[1], secondBin);
	strcat(firstBin, secondBin);
	return strtoul(firstBin, NULL, 2);
/*
  uint8_t field0, field1;
  field0 = field[0];
  field1 = field[1];
  int result = (field0 << 8) | field1;
  return (unsigned int) result;
	//return (unsigned int) ( (field[0] << 8) + field[1] );*/
}


/**
* Takes as a parameter a pointer to the first of 2 bytes of a field specifying length.
*
* Shifts the first byte 8 orders of magnitude to the left and adds it to the value
* of the bit on the right
**/
void convertLengthTo2Bytes(char* ptr, unsigned long length)
{
	ptr[1] = (uint16_t)length;
	ptr[0] = (uint16_t)length >> 8;
/*
	// get the rightmost 8 bits of the length field
	char byte2 = (char) length % 256;

	// get the next 8 bits of the length field
	char byte1 = (char) length / 256;

	ptr[0] = byte1;
	ptr[1] = byte2;*/

}

const char* byte_to_binary(uint8_t x, char* binary)
{
    binary[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(binary, ((x & z) == z) ? "1" : "0");
    }

    return binary;
}

unsigned long retrieveLength(char* packet)
{
		char firstBin[17]; char secondBin[9];
		byte_to_binary(packet[4], firstBin);
		byte_to_binary(packet[5], secondBin);
		strcat(firstBin, secondBin);
		return strtoul(firstBin, NULL, 2);
}

unsigned long receiveResponse(int sock, char* response)
{
	// receive response message from server
	int retrievedLength = 0; // boolean representing whether we have retrieved value from length field
	unsigned long length_Message = 0; // 2 byte field contains length of message (does not count type field)
	int totalBytesRcvd = 0; // total number of bytes received
	for (;;)
	{
		char buffer[BUFFSIZE];
		ssize_t numBytesRcvd = recv(sock, buffer, BUFFSIZE-1, 0);
		//printf("numBytesRcvd: %zu\n", numBytesRcvd); // debugging
		//printf("hello buffer received: %s\n", buffer); // debugging
		if (numBytesRcvd < 0)
			DieWithError("recv() failed");
		else if (numBytesRcvd == 0)
			DieWithError("recv() failed: connection closed prematurely");
		buffer[numBytesRcvd] = '\0'; // append null-character

		// append received buffer to response
		int u;
		for (u = totalBytesRcvd; u < totalBytesRcvd+numBytesRcvd; u++)
		{
			response[u] = buffer[u-totalBytesRcvd];
		}

		// retrieve the length field from message. (located 4th-5th bytes)
		if (!retrievedLength && numBytesRcvd >= 6)
		{
			length_Message = retrieveLength(response);

			retrievedLength = 1;
		}

		// update totalBytesRcvd;
		totalBytesRcvd = totalBytesRcvd + numBytesRcvd;

		printf("totalBytesRcvd: %i\nlength_Message: %lu\n", totalBytesRcvd, length_Message); // DEBUGGING
		printf("packet in receiveResponse: ");
		for (u = 0; u < totalBytesRcvd; u++)
		{
			printf("%c", response[u]);
		}
		printf("\n");

		// if message received is length_Message long, exit the loop
		// 4 for type field, 2 for length field
		if (totalBytesRcvd == 4 + 2 + length_Message)
		{
			printf("ENTERED TOTAL!!!!\n");
			response[totalBytesRcvd] = '\0';
			return length_Message;
		}

	}
}
