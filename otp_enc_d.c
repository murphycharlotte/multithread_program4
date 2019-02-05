/*
*	File: otp_enc_d.c
*	Name: Charlotte Murphy
*	Assignment: VPI Technology Embedded Software Engineer "Homework" 
*	Date: February 4, 2019
*	Desc: A server program to encrypt data received from up to THREADPOOL DEC type clients at a time. 
*		The server receives a plain text file and a cipher key from a client, generates encoded 
*		text based on the input, and sends cipher text back to the client.
*		Based on CS344 assignment, Program4, dated June 9, 2017
*	Usage: For use with encoding: client otp_enc.c, encoding library: encode.c, and key generator: keygen.c
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include<pthread.h>
#include "encode.h"

#define MAXTHREADS	20
#define THREADS	10

void error(const char *msg) { printf("SERVER: error\n");  perror(msg); exit(1); } // Error function used for reporting issues

void *connectionHandler(void *arg);

int main(int argc, char *argv[]) 
{
	int listenSocketFD, establishedConnectionFD, portNumber; 
	socklen_t sizeOfClientInfo; 
	struct sockaddr_in serverAddress, clientAddress;
	if (argc < 2) { fprintf(stderr, "SERVER: USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
																			 
	// Set up the address struct for this process (the server) 
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct 
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string 
	serverAddress.sin_family = AF_INET; // Create a network-capable socket 
	serverAddress.sin_port = htons(portNumber); // Store the port number 
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
																			 
	// Create socket 
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (listenSocketFD < 0) error("SERVER: ERROR opening socket");
	
	// Bind socket to port
	// Enable socket to listen for up to 20 connections
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)  
		error("SERVER: ERROR on binding"); 
	listen(listenSocketFD, MAXTHREADS); 
	
	pthread_t tid[THREADS]; // array of MAXTHREADS thread ids
	pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; // var to protect shared resources
	int tIdx = 0; // set to 0 for first thread
	int newThread = -1; // vars for creating and joining threads
	int join = -1; // for checking pthread_join return value

	while (1) {  
		// Accept a connection, blocking if one is not available until one connects 
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect 
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept 
		if (establishedConnectionFD < 0) error("SERVER: ERROR on accept");
		printf("SERVER: Connected to client\n");
		
		// Create new thread - 
		// params: ptr to thread id, NULL, function to process request, ptr to connection file descriptor-arg for function
		newThread = pthread_create(&tid[tIdx], NULL, connectionHandler, &establishedConnectionFD);	
		if (newThread != 0) error("SERVER: Error on create new thread");
		
		// Increment thread index value for next thread, lock to prevent race condition
		else {
			pthread_mutex_lock(&mutex1); // lock
			tIdx++;
			pthread_mutex_unlock(&mutex1); // unlock
		}

		// If # of threads reaches THREADS, 
		// Lock to prevent race condition
		// iterate through array of thread ids and call join on each thread
		// reset thread index to 0 for subsequent new threads
		if (tIdx >= THREADS) {
			pthread_mutex_lock(&mutex1); // lock
			tIdx = 0;
			while (tIdx < THREADS) {
				join = pthread_join(tid[tIdx], NULL);
				if (join == -1) error("SERVER: ERROR on join");
				tIdx++;
			}
			tIdx = 0;
			pthread_mutex_unlock(&mutex1); //unlock
		}
	}

	close(listenSocketFD); // Close the listening socket 
	return 0;
}


// Function: connectionHandler
// Receives incoming text, validates client type, encodes plain text using client key,
// sends cipher text back to client over socket connection received in param arg
// Subroutines: initServer(), validateMessage(), parseMessage(), encodeMessage(), and invalidClient()
// PRE: Connection established, new thread created, and pointer to connection file descriptor
//		passed in param void *arg
// POST: Incoming message received. If valid client type, message is parsed, encoded,
//		and sent back to client via socket connection.
//		If invalid client type, "NO" is sent back to client via socket connection
void *connectionHandler(void *arg) {
	int tempConnectionFD = *(int*)arg; // gets connection file description
	if (tempConnectionFD < 0) error("SERVER: connectionHandler() ERROR invalid connection file descriptor");

	MESSAGE incoming; //struct for initial string from client
	CIPHER	encoded; //struct for deconstructed incoming message
	initServer(&incoming, &encoded); 	//initialize variables & memory

	// variables for receiving text
	int chunksize = 1024;
	int readSoFar = 0;
	int charsRead = 0;

	// Block socket until data received & continue reading until max bufsize or '&' received
	// Read client's message from the socket while incoming data smaller than buffer 
	// append chars read to incomning.message
	// and while terminating char "&" is not in chunk read
	do {
		charsRead = recv(tempConnectionFD, incoming.message + readSoFar, chunksize, 0);
		readSoFar += charsRead;
		if (charsRead < 0) error("SERVER: ERROR reading from socket");
	} while (readSoFar < incoming.bufsize && strstr(incoming.message, "&") == NULL);

	// ensure client is type ENC and only valid characters were received
	// save plaint text and key to separate strings, generate cipher text
	if (validateMessage(&incoming) == 0) {
		parseMessage(&incoming, &encoded);
		encodeMessage(&encoded);
	}
	else { // reply "NO" if client is not ENC type
		invalidClientType(&encoded);
	}

	// variables for sending text
	int sendBuffer = strlen(encoded.cipherText) + 1; // sendBuffer gets cipherText length + 1
	int sentSoFar = 0;
	int charsWritten = -1;

	// Send encoded cipher text back to the client in chunks until # of sent chars == bufsize
	// if error, close socket, report error, "break;" added just in case
	// accrue # of chars written
	do {
		charsWritten = send(tempConnectionFD, encoded.cipherText + sentSoFar, chunksize, 0);
		if (charsWritten < 0) { close(tempConnectionFD); error("SERVER: ERROR writing to socket"); break; }
		sentSoFar += charsWritten;
	} while (sentSoFar < sendBuffer);

	// sleep(5); // for testing

	// exit thread
	pthread_exit(NULL);
}
