/*
*	File: encode.h
*	Name: Charlotte Murphy
*	Course: CS 344-400
*	Assignment: Program 4
*	Due: June 9, 2017
*	Desc: Header file
*	Usage: For use with encode.c
*/
#pragma once
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>

#ifndef ENCODE
#define ENCODE

char		identifier[6], delim[3], endDelim[3];

typedef struct input {
	char*	fileName;
	char*	line;
	size_t	bufsize;
	size_t	charsEntered;
	int		length;
} INPUT;

typedef struct  message {
	size_t	bufsize;
	char*	message;
	int		length;
} MESSAGE;

typedef struct  cipher {
	size_t	bufsize;
	char*	plainText;
	int		pLength;
	char*	key;
	int		kLength;
	char*	cipherText;
} CIPHER;

// Client functions
void		initClientInput(INPUT* p, char* arg1, INPUT* k, char* arg2);
void		initClientMessages(MESSAGE* c, MESSAGE* e, INPUT* p, INPUT* k);
void		getLineFromFile(INPUT* data);
void		validateInput(INPUT* p, INPUT* k);
void		buildCompleteMessage(MESSAGE* c, INPUT* pInput, INPUT* kInput);
void		validateServerConnection(MESSAGE* e);
void		printMessage(MESSAGE* m);

// Server functions
void		initServer(MESSAGE* in, CIPHER* enc);
int			validateMessage(MESSAGE* in);
void		parseMessage(MESSAGE* in, CIPHER* enc);
void		encodeMessage(CIPHER* enc);
void		invalidClientType(CIPHER* enc);

// Memory management functions
void		cleanupInput(INPUT* i);
void		cleanupMessage(MESSAGE* m);
void		cleanupCipher(CIPHER* c);

#endif // !ENCODE