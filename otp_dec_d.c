/*
*	File: otp_dec_d.c
*	Name: Charlotte Murphy
*	Course: CS 344-400
*	Assignment: Program 4
*	Due: June 9, 2017
*	Desc: **Code is largely from TopHat lecture 4.3 server.c, client.c, and multiserver.c examples**
*		An server program to decrypt data received from up to 5 clients at a time. The server receives 
*		cipher text and a cipher key from a client. It generates decoded text based on the input,
*		and sends the plain text back to the client.
*	Usage: For use with encoding client otp_dec.c
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include "decode.h"

void error(const char *msg) { printf("SERVER: error\n");  perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]) 
{
	MESSAGE incoming; //struct for initial string from client
	CIPHER	decoded; //struct for decontructed incoming message

	initServer(&incoming, &decoded);
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsWritten; 
	socklen_t sizeOfClientInfo; 
	struct sockaddr_in serverAddress, clientAddress;
	if (argc < 2) { fprintf(stderr, "SERVER USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
																			 
	// Set up the address struct for this process (the server) 
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct 
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string 
	serverAddress.sin_family = AF_INET; // Create a network-capable socket 
	serverAddress.sin_port = htons(portNumber); // Store the port number 
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
																			 
	// Set up the socket 
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket 
	if (listenSocketFD < 0) error("SERVER: ERROR opening socket");
	
	// Enable the socket to begin listening 
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port 
		error("ERROR on binding"); 
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	
	while (1) //4.3 multiserver example
	{
		// Accept a connection, blocking if one is not available until one connects 
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect 
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept 
		if (establishedConnectionFD < 0) error("ERROR on accept");
		else (printf("SERVER: connected to client\n"));

		pid_t spawnpid = -5;
		int childExitMethod = -5;

		//fork process if connection is established
		spawnpid = fork();
		switch (spawnpid)
		{
			case -1:
			{
				error("SERVER: fork() failed\n");
				break;
			}
			case 0: //child receives cipher text & cipher key then sends plain text back
			{
				int chunksize = 1024;
				int readSoFar = 0;
				charsRead = 0;
				do
				{
					//Block socket until incoming data received & continue reading until max bufsize or '&' received
					charsRead = recv(establishedConnectionFD, incoming.message + readSoFar, chunksize, 0); // Read the client's message from the socket 
					readSoFar += charsRead;
					if (charsRead < 0) error("SERVER: ERROR reading from socket");
				} while (readSoFar < incoming.bufsize && strstr(incoming.message, "&") == NULL);

				//ensure client is type DEC and only valid characters were received
				if (validateMessage(&incoming) == 0)
				{
					parseMessage(&incoming, &decoded); //save cipher text and key to separate strings
					decodeMessage(&decoded); //generate plain text			
				}
				else //reply if client is not DEC
				{
					decoded.plainText = (char*)malloc(7 * sizeof(char));
					memset(decoded.plainText, '\0', sizeof(decoded.plainText));
					if (decoded.plainText == NULL) { fprintf(stderr, "SERVER: Memory not allocated\n"); exit(1); }
					strcpy(decoded.plainText, "NO");
				}

				strcat(decoded.plainText, endDelim); //add termDelim so client knows when all data is received
				int sendBuffer = strlen(decoded.plainText) + 1;

				int sentSoFar = 0;
				charsWritten = -5;
				do
				{
					// Send encoded plain text back to the client in chunks until sent chars == bufsize
					charsWritten = send(establishedConnectionFD, decoded.plainText + sentSoFar, chunksize, 0); // Send success back 
					if (charsWritten < 0) { error("SERVER: ERROR writing to socket"); close(establishedConnectionFD); break; } // Close the existing socket which is connected to the client
					sentSoFar += charsWritten;
				} while (sentSoFar < sendBuffer);

				//exit process once completed - instead of break 
				exit(0);
			}
			default: //parent calls wait/no hang
			{
				waitpid(spawnpid, &childExitMethod, WNOHANG);
				break;
			}
		}
	}

	close(listenSocketFD); // Close the listening socket 
	return 0;
}