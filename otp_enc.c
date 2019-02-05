/*
*	File: otp_enc.c
*	Name: Charlotte Murphy
*	Assignment: VPI Technology Embedded Software Engineer "Homework"
*	Date: February 4, 2019
*	Desc: A client program to transmit plain text and a cipher key to ENC type server 
*		and receive encoded plain text as cipher text from ENC server.
*		Based on CS344 assignment, Program4, dated June 9, 2017
*	Usage: For use with encoding server: otp_enc_d.c, encoding library: encode.c, and key generator: keygen.c
*/
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include "encode.h"

#define CHUNKSIZE	1024

void error(const char *msg) { printf("CLIENT: error\n");   perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]) 
{
	// pInput gets plain text, kInput gets key
	// complete gets plaint text and key, encoded gets cipher text
	INPUT pInput, kInput;
	MESSAGE complete, encoded;

	initClientInput(&pInput, argv[1], &kInput, argv[2]);

	int socketFD, portNumber;
	struct sockaddr_in serverAddress; 
	struct hostent* serverHostInfo; 

	// Check usage & args
	if (argc < 4) { fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); } 
																					  
	// Set up the server address struct 
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct 
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string 
	serverAddress.sin_family = AF_INET; // Create a network-capable socket 
	serverAddress.sin_port = htons(portNumber); // Store the port number 
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address 
	if (serverHostInfo == (NULL)) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(2); } 
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Create socket 
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0) error("CLIENT: ERROR opening socket"); 

	// Connect to server address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
		error("CLIENT: ERROR connecting");
	
	// get plain text and key from respective files
	// ensure text is received from ENC type
	// initialize messages
	// concatenate text to build complete message
	getLineFromFile(&pInput);
	getLineFromFile(&kInput);
	validateInput(&pInput, &kInput);
	initClientMessages(&complete, &encoded, &pInput, &kInput);
	buildCompleteMessage(&complete, &pInput, &kInput);
	
	// Send plain text and key to server 
	// Set sendBuffer size to message length + 1
	int sentSoFar = 0;
	int charsWritten = -1;
	int sendBuffer = complete.length + 1;
	
	// Send CHUNKSIZE chars at a time up to length of complete.message
	do {
		charsWritten = send(socketFD, complete.message + sentSoFar, CHUNKSIZE, 0); 	// Write to the server 
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		sentSoFar += charsWritten;
	} while (sentSoFar < sendBuffer);

	// Receive encoded message from server 
	int readSoFar = 0;
	int charsRead = -1;
	int readBuffer = sendBuffer - kInput.length; //approx length of send buffer - length of key, bc only receiving cipher text

	// Read CHUNKSIZE chars at a time from socket leaving \0 at end 
	// append chars read to encoded.message
	// read until readSoFar >= readBuffer or "&" received in chunk
	do {
		charsRead = recv(socketFD, encoded.message + readSoFar, CHUNKSIZE, 0); 
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		readSoFar += charsRead;
	} while ((readSoFar < readBuffer) && (strstr(encoded.message, "&") == NULL));

	// Ensure correct server type
	// Print encoded message without trailing term character
	validateServerConnection(&encoded);
	printMessage(&encoded);

	// close socket
	close(socketFD); 

	return 0;
}


/*	Additional Sources for original assignment date June 2017
*		TopHat lecture 4.3 server.c, client.c, and multiserver.c examples
*		https://www.daniweb.com/programming/software-development/code/216411/reading-a-file-line-by-line
*		http://www.sanfoundry.com/c-program-size-file-handling/
*		http://www.crasseux.com/books/ctutorial/getline.html
*		https://cboard.cprogramming.com/c-programming/134497-socket-programming-send-loop.html
*		https://stackoverflow.com/questions/32390478/what-is-the-return-value-of-strcspn-when-str1-does-not-contain-str2
*		http://www.c4learn.com/c-programming/c-arithmetic-operations-on-character/
*/